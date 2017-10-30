package ex1;

public class ParallelGameOfLife implements GameOfLife {
    int threadCount;
    boolean[][][] result;

    ParallelGameOfLife() {
		/* thread count will be used to create a barrier. we want to swap the fields only once all the threads finished
		   calculating the current field.*/
        threadCount = 0;
        threadLimit = 0;
		/* result will hold 2 boards, 0 is the current generation and 1 is the next. */
        result = null;
    }
    private void copyBoard(boolean[][] dst, boolean[][] src ) {
        for (int row = 0; row < src.length; row++){
            for (int col = 0; col < src[0].length; col++){
                dst[row][col] = src[row][col]
            }
        }
    }
    private void allocateBoard(boolean[][] newBoard, boolean sizes[][]) {
        newBoard = new boolean[sizes.length][];
        for (int i = 0; i < sizes.length; i++){
            newBoard = new sizes[0].length
        }
    }
    private void initBoards(boolean[][] initalField) {
        this.result = new boolean[2][][];
        allocateBoard(result[0], initalField);
        allocateBoard(result[1], initalField);
        copyBoard(result[0], initalField);
    }
    private void swapBoards(){
        boolean[][] tmp = result[0];
        result[0] = result[1];
        result[1] = tmp;
    }
    public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit,
                                int generations) {
        // TODO: input check.

        // TODO: intialize the boards (2 for the starting)
        this.threadLimit = hSplit * vSplit;
        initBoards(initalField);

        int intervalY = initalField.length/vSplit, intervalX = initalField[0].length/hSplit;
        // TODO: 1) calc the amount of threads for the barrier 2) calc each threads dimentions 3)intialize the threads.
        for (int minY=0, countY=0; countY<vSplit; minY+=intervalY, countY++){
            int maxY=(countY!=vSplit-1)?minY + intervalY:initalField.length;
            for (int minX=0, countX=0; counX<hSplit; minX+=intervalX, countX++) {
                int maxX = (countX != hSplit - 1) minX + intervalX:initalField[0].length;
                // TODO: create a container for manipulating the threads. add each thread to the container.
                CalcArea(minX, maxX, minY, maxY, generations);
            }
        }
        // TODO: initialize barrier. wait while thread count < thread limit, maybe a flag that they can run.
        // TODO: Create a loop that waits for all threads, and then notifies them that they can start the net generation
        // 		 the loop will swap the boards, and then notify the threads.
        // TODO: Return 2 boards.
        return null;
    }
    class CalcArea implements Runnable {
        /* Each thread should calculate its own area, using the main boards.*/
        int minX, maxX, minY, maxY, currentGen, requiredGen;
        boolean stop = false;

        CalcArea(int minX, int maxX, int minY, int maxY, int requiredGen){
            this.minX = minX;
            this.maxX = maxX;
            this.minY = minY;
            this.maxY = maxY;
            this.currentGen = 0;
            this.requiredGen = requiredGen;
        }

        private void clacSquare(int row, int col) {
            if (row < minY || row > maxY || col < minX || col > maxX ) {
                // throw an exception so we can debug easily.
            }
            int aliveNiehbors = 0;
            // aliveNiehbors = (result[row+1][col])?(aliveNiehbors + 1):(aliveNiehbors);
            // case 1: not the top edge, we can look up.
            if (row != 0){
                aliveNiehbors = (result[0][row-1][col])?(aliveNiehbors + 1):(aliveNiehbors);
                // case 2: look up and to the left
                if (col != 0) {
                    aliveNiehbors = (result[0][row-1][col-1])?(aliveNiehbors + 1):(aliveNiehbors);
                }
                // case 3: look up and to the right:
                if (col != result[0][0].length) {
                    aliveNiehbors = (result[0][row-1][col+1])?(aliveNiehbors + 1):(aliveNiehbors);
                }
            }
            // case 4: not the bottom edge, can look down
            if (row != result[0].length) {
                aliveNiehbors = (result[0][row+1][col])?(aliveNiehbors + 1):(aliveNiehbors);
                // case 5: look to the bottom and to the left.
                if (col != 0) {
                    aliveNiehbors = (result[0][row-1][col-1])?(aliveNiehbors + 1):(aliveNiehbors);
                }
                // case 6: look to the bottom and to the right.
                if (col != result[0][0].length){
                    aliveNiehbors = (result[0][row-1][col+1])?(aliveNiehbors + 1):(aliveNiehbors);
                }
            }
            // case 7: look to the right
            if (col != result[0][0].length){
                aliveNiehbors = (result[0][row][col+1])?(aliveNiehbors + 1):(aliveNiehbors);
            }
            // case 8: look to the left
            if (col != 0) {
                aliveNiehbors = (result[0][row][col-1])?(aliveNiehbors + 1):(aliveNiehbors);
            }
            if (!result[0][row][col]){
                result[1][row][col] = (aliveNiehbors==3)
            } else {
                result[1][row][col] = (aliveNiehbors==3) || (aliveNiehbors==2)
            }
        }

        public void halt() { this.stop = True; }

        @Override public void run(){
            while(!this.stop && this.currentGen<this.requiredGen){
                this.currentGen++;
                // calculates the next generation for each cell in the field that belongs to this thread.
                for (int row = this.minY; row < this.maxY; row++){
                    for (int col = this.minX; col < this.maxX; col++){
                        clacSquare(row, col);
                    }
                }
                // TODO: add the barrier here, increas a sinc conter.
            }
        }

    }
}
