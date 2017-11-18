package ex1;


import java.util.ArrayList;

public class ParallelGameOfLife implements GameOfLife {

	final int AMOUNT_OF_RESULTS = 2;
	Field [][] threadLocations;
	int hSplit;

	public Field[][] getThreadLocations() {
		return threadLocations;
	}

	public int gethSplit() {
		return hSplit;
	}

	public void sethSplit(int hSplit) {
		this.hSplit = hSplit;
	}

	public int getvSplit() {
		return vSplit;
	}

	public void setvSplit(int vSplit) {
		this.vSplit = vSplit;
	}

	int vSplit;

	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit,
			int generations) {
		this.hSplit = hSplit;
		this.vSplit = vSplit;
		// allocate a 3D field for the result.
		boolean[][][] gofResult = allocateGofResult(initalField);
		// create hSplit*vSplit runnable objects.
		allocateAndCreateFields(initalField, gofResult, generations);
		// notify each runnable who are his neighbors are.
		applyNeighbors();
		// create, start and wait for each thread to calculate his runnable.
		startGOF();

		return gofResult;
	}

	/*
	 * Used to allocate a 3D array for the threads Global result.
	 * Each thread will write only to his cells, and only once all computations are done.
	 * Even though this memory is shared by all the threads, according to the exercise FAQ it is ok.
	 * (in fact it is faster if each thread writes his own results to the correct location)
	 * Param initalField - the initial state of the game.
	 */
	public boolean[][][] allocateGofResult(boolean[][] initalField) {
		boolean[][][] gofResult = new boolean[AMOUNT_OF_RESULTS][][];
		for (int i = 0; i < AMOUNT_OF_RESULTS; i++){
			gofResult[i] = new boolean[initalField.length][];
			for ( int row = 0; row < initalField.length; row++){
				gofResult[i][row] = new boolean[initalField[0].length];
			}
		}
		return gofResult;
	}

	/*
	 * Use to allocate and create the Fields that the threads will run.
	 * Param initialField - the initial state of the field in generation 0.
	 * 		 gofResult - a global 3D boolean field for the threads final results.
	 * 		 generations - the amount of generations that need to be computed.
	 * return Field[vSplit][hSplit].
	 */
	public void allocateAndCreateFields(boolean[][] initialField, boolean[][][] gofResult, int generations) {
		int minX , maxX = 0, minY = 0, maxY = 0;
		int intervalY = initialField.length / vSplit;
		int intervalX = initialField[0].length / hSplit;
		this.threadLocations  = new Field[vSplit][];

		for ( int row = 0; row < vSplit; row++) {
			threadLocations[row] = new Field[hSplit];
			minX = 0;
			maxX = 0;
			maxY =  (row != vSplit-1) ? maxY + intervalY : initialField.length;
			for (int col = 0; col < hSplit; col++) {
				maxX = (col != hSplit - 1) ? maxX + intervalX : initialField[0].length;
				threadLocations[row][col] = new Field(initialField, minY, maxY-1, minX, maxX-1, generations, gofResult);
				minX = maxX;
			}
			minY = maxY;
		}
	}

	/*
	 * Use to notify each Field which threads are his neighbors for communicating and applying the producer consumer
	 * algorithm.
	 */
	public void applyNeighbors(){
		Field up, upRight, right, downRight, down, downLeft, left, upLeft;
		for (int row = 0; row < vSplit; row++) {
			for (int col = 0; col < hSplit; col++){
				if ( row == 0) {
					up = null;
					upRight = null;
					upLeft = null;
				} else {
					up = threadLocations[row-1][col];
					upLeft = (col > 0) ? threadLocations[row-1][col-1] : null;
					upRight = (col < hSplit-1) ? threadLocations[row-1][col+1] : null;
				}
				if (row == vSplit-1) {
					down = null;
					downLeft = null;
					downRight = null;
				} else {
					down = threadLocations[row+1][col];
					downLeft = (col > 0) ? threadLocations[row+1][col-1] : null;
					downRight = (col < hSplit-1) ? threadLocations[row+1][col+1] : null;
				}
				left = (col > 0) ? threadLocations[row][col-1] : null;
				right = (col < hSplit-1) ? threadLocations[row][col+1] : null;
				Neighbors n = new Neighbors(up, upRight, right, downRight, down, downLeft, left, upLeft);
				threadLocations[row][col].setNeighbors(n);
			}
		}
	}

	/*
	 * startGOF will create a thread for each field, start the thread, and wait for all the threads to finnish.
	 */
	public void startGOF(){
		ArrayList<Thread> threads = new ArrayList<Thread>(hSplit*vSplit);
		for (int row = 0; row < vSplit; row++) {
			for (int col = 0; col < hSplit; col++) {
				threads.add(new Thread(threadLocations[row][col]));
			}
		}
		for (Thread t : threads)
			t.start();

		for (Thread t : threads) {
			try {
				t.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	public static void printBoard(boolean[][] board) {

		for (int i = 0; i < board.length; i++) {
			System.out.println("");
			for (int j = 0; j < board[0].length; j++){
				System.out.print(board[i][j]? 1 : 0);
			}
		}
		System.out.println("");
	}
}