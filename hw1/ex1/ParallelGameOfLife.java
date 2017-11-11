package ex1;

import java.util.ArrayList;

//-----------------------------------------------------------------------------
//                              Main Class
//-----------------------------------------------------------------------------
public class ParallelGameOfLife implements GameOfLife {

	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit,
			int generations) {

	    boolean[][][] x=new boolean[2][][];	
        ArrayList<ArrayList<ArrayList<Boolean>>> field;

        // give each thread a parthial-copy of the initalField
        field = divideField(initalField, hSplit, vSplit);

		x[0] = get_gen(initalField,hSplit,vSplit, generations-1);
		x[1] = get_gen(initalField,hSplit,vSplit, generations);
        
		return x;
	}

	private boolean[][] get_gen(boolean[][] initalField, int hSplit, int vSplit, int generations) {
	
		// init the field -> copy the field to the local array
		boolean[][] input=new boolean[initalField.length][];
		
		for (int i=0;i<initalField.length;i++){
			input[i]=new boolean[initalField[0].length];
			for (int k=0;k<input[i].length;k++){
				input[i][k]=initalField[i][k];
			}
		}

		boolean[][] result=new boolean[initalField.length][];
		
		for (int g=0;g<generations;g++){
			for (int i=0;i<initalField.length;i++){
				if (result[i]==null){
					// using first time -> copy the array
					result[i]=new boolean[initalField[i].length];
				}
				
				for (int j=0;j<initalField[i].length;j++){
					int numNeighbors=numNeighbors(i,j,input);
					result[i][j]=false;
					if (numNeighbors==3 || (input[i][j]&& numNeighbors==2)){
						result[i][j]=true;
					}
				}
			}
			boolean[][] tmp;
			tmp=input;
			input=result;
			result=tmp;
		}
		return input;
	}
	
	private int numNeighbors(int x,int y, boolean[][] field ){
		int counter=(field[x][y]?-1:0);
		for (int i=x-1; i<=x+1;i++ ){
			if (i<0||i>=field.length){ continue ; }
			for (int j=y-1; j<=y+1;j++){
				if (j<0||j>=field[0].length){ continue ; }
				counter+=(field[i][j]?1:0);
			}
		}
		return counter;
	}

    // FIXME: update to 2D array if autorized
    // this function create an ArrayList for all threads
    // each object in the ArrayList is a 2D ArrayList (a part of the field)
    // we cannot use an array of 2D ArraysLists cause arrays don't hold
    //                                                       generic types
    private ArrayList<ArrayList<ArrayList<Boolean>>> 
    divideField(boolean[][] initalField, int hSplit, int vSplit) {
        
        int numOfThread = hSplit * vSplit;

        // create the ArrayList
        ArrayList<ArrayList<ArrayList<Boolean>>> fields = new ArrayList<>();

        // for each thread copy its relevant part of initalField
        for (int i = 0 ; i < numOfThread ; i++) 
            fields.add(copyFromOrigin(initalField, i, hSplit, vSplit));
        
        return fields;
    }

    // FIXME: update to 2D array if autorized
    private ArrayList<ArrayList<Boolean>> 
    copyFromOrigin(boolean[][] initalField, int threadIndex, int hSplit, 
            int vSplit) {

        int hSplitSize = initalField[0].length / hSplit; 
        int vSplitSize = initalField.length / vSplit;
        
        int row = (threadIndex / hSplit ) * vSplitSize; 
        int col = (threadIndex % hSplit) * hSplitSize; 

        // if the field didn't split equally so the remider is for the last one
        int rowReminder = initalField.length % vSplit;
        if (rowReminder != 0 && row / vSplitSize == vSplit-1) {
            vSplitSize += rowReminder;
        }

        // if the field didn't split equally so the remider is for the last one
        int colReminder = initalField[0].length % hSplit;
        if (colReminder != 0 && threadIndex % hSplit == hSplit-1) {
            hSplitSize += colReminder;
        }

        ArrayList<ArrayList<Boolean>> field = new ArrayList<>();
        for (int i = 0 ; i < vSplitSize ; i++) {
            field.add(new ArrayList<Boolean>());
            for (int j = 0 ; j < hSplitSize ; j++) {
                field.get(i).add(initalField[row + i][col + j]);
            }
        }
        return field;
    }

    // FIXME: for testing Cell class - remove when done
    public static void main(String args[]) {
        Cell c = new Cell(true, 3);
        c.cellMain();
    }

}



//-----------------------------------------------------------------------------
//                              Field Class
//-----------------------------------------------------------------------------

//class Field implements Runnable {
//    //FIXME: implement
//}




//-----------------------------------------------------------------------------
//                              MyQueu Class
//-----------------------------------------------------------------------------

//class MyQueue {
//    
//    private ArrayList<Cell> queue;
//
//    synchronized Cell dequeue() {
//
//    }
//
//    void enqueue(Cell c) {
//
//    }
//}





//-----------------------------------------------------------------------------
//                              Cell Class
//-----------------------------------------------------------------------------

class Cell {

    private boolean isAlive;
    private int gen;
    private int checkedNeighbors;
    private int aliveNeighbors;
    // not all cells have 8 neighbors
    private int maxNeighbors;        
    // a copy of the old version for reading 
    private Cell oldVersion;

    Cell(boolean isAlive, int maxNeighbors) {
        this.isAlive = isAlive;
        gen = 0;
        checkedNeighbors = 0;
        aliveNeighbors = 0;
        this.maxNeighbors = maxNeighbors;
        // oldVersion is initiallize no null by default
    }

    // given a neighbor cell we can add it to the calculation for our cell
    void addNeighbor(Cell c) {

        //FIXME: for debugging - remove when done
        if (gen != c.gen)
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());

        checkedNeighbors++;
        if (c.isAlive) {
            aliveNeighbors++;
        }

        // if we handlend all neighbors we can update the cell to next gen
        if (checkedNeighbors == maxNeighbors) {
            moveToNextGen();
        }
    }
    
    private void moveToNextGen() {

        // FIXME: just for debugging - remove when finish
        if (aliveNeighbors > checkedNeighbors) 
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());

        // update oldVersion to current version
        // NOTE: we update only relevant data for reading
        if (oldVersion == null) {
            // oldVersion not yet allocated
            oldVersion = new Cell(isAlive, maxNeighbors);
            // oldVersion.oldVersion recieve null by default so no loop
        }
        oldVersion.gen = gen;

        // update current version
        gen++;
        if (aliveNeighbors == 3 || (aliveNeighbors == 2 && isAlive) ) 
            isAlive = true;
        else 
            isAlive = false;
        
        checkedNeighbors = 0;
        aliveNeighbors = 0;
    }

    //FIXME: remove all -----------------------------------------------------
    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

    static void cellMain() {

        System.out.print("CellTest()...");

       // // test that my debugging assert works
       // Cell badCell = new Cell(true, 4);
       // badCell.gen = 5;
       // Cell c0 = new Cell(false, 4);
       // c0.addNeighbor(badCell);
        
        Cell c1 = new Cell(false, 4);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 
        test(c1.maxNeighbors == 4); 
        test(c1.oldVersion == null); 

        Cell deadCell = new Cell(false, 8);
        Cell liveCell = new Cell(true, 8);

        c1.addNeighbor(liveCell);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 1); 
        test(c1.aliveNeighbors == 1); 
        test(c1.oldVersion == null); 

        c1.addNeighbor(deadCell);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 2); 
        test(c1.aliveNeighbors == 1); 
        test(c1.oldVersion == null); 

        c1.addNeighbor(liveCell);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 3); 
        test(c1.aliveNeighbors == 2); 
        test(c1.oldVersion == null); 

        c1.addNeighbor(liveCell);
        test(c1.isAlive == true); 
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 
        test(c1.oldVersion != null); 

        liveCell.gen++;
        deadCell.gen++;

        c1.addNeighbor(liveCell);
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 1); 
        test(c1.aliveNeighbors == 1); 
        test(c1.oldVersion != null); 

        c1.addNeighbor(liveCell);
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 2); 
        test(c1.aliveNeighbors == 2); 
        test(c1.oldVersion != null); 

        c1.addNeighbor(deadCell);
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 3); 
        test(c1.aliveNeighbors == 2); 
        test(c1.oldVersion != null); 

        c1.addNeighbor(deadCell);
        test(c1.isAlive == true);
        test(c1.gen == 2); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 

        liveCell.gen++;
        deadCell.gen++;

        c1.addNeighbor(liveCell);
        c1.addNeighbor(deadCell);
        c1.addNeighbor(deadCell);
        c1.addNeighbor(deadCell);
        test(c1.isAlive == false);
        test(c1.gen == 3); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 

        System.out.println("[OK]");
    }
    //FIXME: ----------------------------------------------------------------
}
























