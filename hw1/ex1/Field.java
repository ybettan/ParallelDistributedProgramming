package ex1;

import java.util.Vector;


public class Field implements Runnable {

    private Cell3D[][] field;
    private SyncedQueue queue;
    private int numOfDoneCells;
    private int generations;
    private Neighbors neighbors; 
    private boolean[][][] result; 
    private final int CORNER_MAX_NEIGHBORS = 3;
    private final int SIDE_MAX_NEIGHBORS = 5;
    private final int INTERNAL_MAX_NEIGHBORS = 8;
    

    public Field(boolean[][] initalField, int minI, int maxI, int minJ, 
            int maxJ, int generations, boolean[][][] result) {
        this.field = createPartialCopy(initalField, minI, maxI, minJ, maxJ);
        queue = new SyncedQueue();
        numOfDoneCells = 0;
        this.generations = generations;
        this.result = result;
    }
    
    /* receive as input all Field neighbors, Field is the owner of this data */
    public void setNeighbors(Neighbors neighbors) {
        this.neighbors = neighbors;
    }

    /* get a pointer to the queue for other Fields to enqueue data to it */
    public SyncedQueue getQueue() {
        return queue;
    }

    @Override public void run() {
        autonomusPart();        
        communicationPart();        
        writeResult();
    } 

//-----------------------------------------------------------------------------
//                      FIXME:DEBUGGING - remove
//-----------------------------------------------------------------------------

    public  void printField() {
        for (int i=0 ; i<field.length ; i++) {
            for (int j=0 ; j<field[0].length ; j++) {
                boolean b = field[i][j].getCurrentCopy().isAlive();
                System.out.print((b ? "t " : "f "));
            }
            System.out.println();
        }
        System.out.println();
    }

    public Neighbors getNeighbors() {
        return neighbors;
    }

//-----------------------------------------------------------------------------
//                              private methods
//-----------------------------------------------------------------------------

    private Cell3D[][] createPartialCopy(boolean[][] initalField, int minI, 
            int maxI, int minJ, int maxJ) {

        int numOfRows = maxI - minI + 1;
        int numOfCols = maxJ - minJ + 1;
        int buttomBoundry = initalField.length;
        int rightBoundry = initalField[0].length;
        Cell3D[][] res = new Cell3D[numOfRows][];
        for (int i = minI ; i <= maxI ; i++) {
            res[i-minI] = new Cell3D[numOfCols];
            for (int j = minJ ; j <= maxJ ; j++) {
                boolean isAlive = initalField[i][j];
                int maxNeighbors;
                if (isCorner(numOfRows-1, numOfCols-1, i, j)) {
                    maxNeighbors = CORNER_MAX_NEIGHBORS;
                } else if (isSideButNotCorner(numOfRows-1, numOfCols-1, i, j)) {
                    maxNeighbors = SIDE_MAX_NEIGHBORS;
                } else {
                    maxNeighbors = INTERNAL_MAX_NEIGHBORS;
                } 
                res[i-minI][j-minJ] = new Cell3D(isAlive, maxNeighbors, i, j);
            }
        }
        return res;
    }

    private boolean isCorner(int maxRow, int maxCol, int i, int j) {
        boolean res = false;

        if (isUpRightCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        if (isDownRightCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        if (isDownLeftCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        if (isUpLeftCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        return res;
    }

    private boolean isUpRightCorner(int maxRow, int maxCol, int i, int j) {
        return (i == 0 && j == maxCol); 
    }

    private boolean isDownRightCorner(int maxRow, int maxCol, int i, int j) {
        return (i == maxRow && j == maxCol); 
    }

    private boolean isDownLeftCorner(int maxRow, int maxCol, int i, int j) {
        return (i == maxRow && j == 0); 
    }

    private boolean isUpLeftCorner(int maxRow, int maxCol, int i, int j) {
        return (i == 0 && j == 0); 
    }

    private boolean isSideButNotCorner(int maxRow, int maxCol, int i, int j) {
        boolean res = false;

        if (isUpSideButNotCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        if (isRightSideButNotCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        if (isDownSideButNotCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        if (isLeftSideButNotCorner(maxRow, maxCol, i, j)) {
            res = true;
        }
        return res;
    }

    private boolean isUpSideButNotCorner(int maxRow, int maxCol, int i, int j) {
        return ( i == 0 && !isCorner(maxRow, maxCol, i, j) );
    }

    private boolean isRightSideButNotCorner(int maxRow, int maxCol, int i, int j) {
        return ( j == maxCol && !isCorner(maxRow, maxCol, i, j) );
    }

    private boolean isDownSideButNotCorner(int maxRow, int maxCol, int i, int j) {
        return ( i == maxRow && !isCorner(maxRow, maxCol, i, j) );
    }

    private boolean isLeftSideButNotCorner(int maxRow, int maxCol, int i, int j) {
        return ( j == 0 && !isCorner(maxRow, maxCol, i, j) );
    }

    private void autonomusPart() {
        sendMargins();
        buildPyramid(0, field.length, 0, field[0].length, 0);
    }

    /*
     * send the margin cells to the neighbors.
     */
    private void sendMargins() {
        for (int row=0; row < field.length; row++) {
            for (int col=0; col < field[0].length; col++){
                if ((row == 0) || (row==field.length-1) || (col==0) || (col==field[0].length-1))
                    sendToNeighbors(field[row][col].getCurrentCopy(),row,col);
            }
        }
    }
    /*
     * build a recursive pyramid of generations. each time the function will compute the generation for the given base
     * which the dimensions limited by: rows start at min row and end at max row.
     *                                  columns start at min col and end at max col.
     * next the the function will calculate the new dimensions for the next base and call it self again.
     * each time the generation that is passed should increase by 1.
     * the recursion will stop if:
     *          1) the base is to small to compute.
     *          2) if the max generation was calculated.
     *          3) if no cell advanced a generation during
     */
    private void buildPyramid(int minRow, int maxRow, int minCol, int maxCol, int currentGeneration) {
        // stopping conditions.
        if (currentGeneration == generations) {
            numOfDoneCells = (maxCol-minCol) * (maxRow-minRow);
            return;
        }
        if ((minRow>=maxRow) && (minCol>=maxCol))
            return;
        // limits for next iteration.
        int nextMinRow = -1, nextMaxRow = -1, nextMinCol = -1, nextMaxCol = -1;

        for (int row=minRow; row < maxRow; row++){
            for (int col=minCol; col < maxCol; col++) {
                // look up:
                if (row > minRow) {
                    field[row][col].addNeighbor(field[row-1][col].getByGen(currentGeneration));
                    // look up and left
                    if (col>minCol)
                        field[row][col].addNeighbor(field[row-1][col-1].getByGen(currentGeneration));
                    // look up and right
                    if (col < maxCol-1)
                        field[row][col].addNeighbor(field[row-1][col+1].getByGen(currentGeneration));
                }
                // look down
                if (row < maxRow) {
                    field[row][col].addNeighbor(field[row+1][col].getByGen(currentGeneration));
                    // look down and right
                    if (col < maxCol-1)
                        field[row][col].addNeighbor(field[row+1][col+1].getByGen(currentGeneration));
                    // look down and left
                    if (col>minCol)
                        field[row][col].addNeighbor(field[row+1][col-1].getByGen(currentGeneration));
                }
                // look left
                if (col>minCol)
                    field[row][col].addNeighbor(field[row][col-1].getByGen(currentGeneration));
                // look right
                if (col < maxCol)
                    field[row][col].addNeighbor(field[row][col+1].getByGen(currentGeneration));
                // find the first cell that was updated. he is the top left corner of the pyramid above.
                if ((nextMinCol == -1) && (field[row][col].getByGen(currentGeneration+1) != null)) {
                    nextMinCol = col;
                    nextMinRow = row;
                }
                // find the last cell that was updated. he is the bottom left corner of the pyramid above.
                if (field[row][col].getByGen(currentGeneration+1) != null) {
                    nextMaxCol = col;
                    nextMaxRow = row;
                }
            }
        }
        buildPyramid(nextMinRow, nextMaxRow, nextMinCol, nextMaxCol, currentGeneration+1);
    }

    /* update all relevant neighbors recursively for each Cell in queue */
    private void communicationPart() {

        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        /* while not all cells have arrived to required generation */
        while (numOfDoneCells < (maxCol+1) * (maxRow+1)) {
           Cell cellFromOtherThread = queue.dequeue();
           recursiveAddNeighbors(cellFromOtherThread);
        }
    }

    private void sendToNeighbors(Cell c, int i, int j) {
        Field neighbor;
        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        /* up neighbor case*/
        neighbor = neighbors.getUp();
        if (neighbor != null && i == 0) {
           neighbor.getQueue().enqueue(c);
        }
        /* up right neighbor case*/
        neighbor = neighbors.getUpRight();
        if (neighbor != null && i == 0 && j == maxCol) {
           neighbor.getQueue().enqueue(c);
        }
        /* right neighbor case*/
        neighbor = neighbors.getRight();
        if (neighbor != null &&  j == maxCol) {
           neighbor.getQueue().enqueue(c);
        }
        /* down right neighbor case*/
        neighbor = neighbors.getDownRight();
        if (neighbor != null &&  i == maxRow && j == maxCol) {
           neighbor.getQueue().enqueue(c);
        }
        /* down neighbor case*/
        neighbor = neighbors.getDown();
        if (neighbor != null &&  i == maxRow) {
           neighbor.getQueue().enqueue(c);
        }
        /* down left neighbor case*/
        neighbor = neighbors.getDownLeft();
        if (neighbor != null &&  i == maxRow && j == 0) {
           neighbor.getQueue().enqueue(c);
        }
        /* left neighbor case*/
        neighbor = neighbors.getLeft();
        if (neighbor != null && j == 0) {
           neighbor.getQueue().enqueue(c); 
        }
        /* up left neighbor case*/
        neighbor = neighbors.getUpLeft();
        if (neighbor != null && i == 0 && j == 0) {
           neighbor.getQueue().enqueue(c);
        }
    }

    private void recursiveAddNeighbors(Cell cellFromOtherThread) {

        Vector<Cell3D> dependencies = getDependecies(cellFromOtherThread);    

        for (Cell3D dep : dependencies) {
            dep.addNeighbor(cellFromOtherThread);
            if (dep.wasUpdated()) {
                if (dep.getCurrentCopy().getGen() == generations) {
                    numOfDoneCells++;
                }
                recursiveAddNeighbors(dep.getCurrentCopy());
            }
        }

    }

    private Vector<Cell3D> getDependecies(Cell cellFromOtherThread) {

        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        /* find all the Cell3D that needs that cellFromOtherThread */ 
        int globalIFromOtherThread = cellFromOtherThread.getGlobalI();
        int globalJFromOtherThread = cellFromOtherThread.getGlobalJ();

        Vector<Cell3D> res = new Vector<>();

        /* cellFrom another thread is a perfect corner so one of the 4 field 
         * corners is res */

        /* only up right corner needs it */
        if (isUpRightCorner(maxRow, maxCol, globalIFromOtherThread+1, 
                    globalJFromOtherThread-1)) {
            res.addElement(field[0][maxCol]);
        }
        /* only down right corner needs it */
        else if (isDownRightCorner(maxRow, maxCol, globalIFromOtherThread-1, 
                    globalJFromOtherThread-1)) {
            res.addElement(field[maxRow][maxCol]);
        }
        /* only down left corner needs it */
        else if (isDownLeftCorner(maxRow, maxCol, globalIFromOtherThread-1, 
                    globalJFromOtherThread+1)) {
            res.addElement(field[maxRow][0]);
        }
        /* only up left corner needs it */
        else if (isUpLeftCorner(maxRow, maxCol, globalIFromOtherThread+1, 
                    globalJFromOtherThread+1)) {
            res.addElement(field[0][0]);
        }

        /* cellFrom another thread is a NOT perfect corner so one of the 4
         * corner and another one next to it are res */

        /* up right NOT perfect corner case - 2 cases */
        else if (isUpSideButNotCorner(maxRow, maxCol,globalIFromOtherThread+1,
                    globalJFromOtherThread-1)){
            res.addElement(field[0][maxCol]);
            res.addElement(field[0][maxCol-1]);
        }
        else if (isRightSideButNotCorner(maxRow, maxCol,globalIFromOtherThread+1,
                    globalJFromOtherThread-1)){
            res.addElement(field[0][maxCol]);
            res.addElement(field[1][maxCol]);
        }
        /* down right NOT perfect corner case - 2 cases */
        else if (isRightSideButNotCorner(maxRow, maxCol,globalIFromOtherThread-1,
                    globalJFromOtherThread-1)){
            res.addElement(field[maxRow][maxCol]);
            res.addElement(field[maxRow-1][maxCol]);
        }
        else if (isDownSideButNotCorner(maxRow, maxCol,globalIFromOtherThread-1,
                    globalJFromOtherThread-1)){
            res.addElement(field[maxRow][maxCol]);
            res.addElement(field[maxRow][maxCol-1]);
        }
        /* down left NOT perfect corner case - 2 cases */
        else if (isDownSideButNotCorner(maxRow, maxCol,globalIFromOtherThread-1,
                    globalJFromOtherThread+1)){
            res.addElement(field[maxRow][0]);
            res.addElement(field[maxRow][1]);
        }
        else if (isLeftSideButNotCorner(maxRow, maxCol,globalIFromOtherThread-1,
                    globalJFromOtherThread+1)){
            res.addElement(field[maxRow][0]);
            res.addElement(field[maxRow-1][0]);
        }
        /* up left NOT perfect corner case - 2 cases */
        else if (isLeftSideButNotCorner(maxRow, maxCol,globalIFromOtherThread+1,
                    globalJFromOtherThread+1)){
            res.addElement(field[0][0]);
            res.addElement(field[1][0]);
        }
        else if (isUpSideButNotCorner(maxRow, maxCol,globalIFromOtherThread+1,
                    globalJFromOtherThread+1)){
            res.addElement(field[0][0]);
            res.addElement(field[0][1]);
        }


        /* cellFrom another thread is not a corner so it has 3 res */

        /* up case */
        else if (isUpSideButNotCorner(maxRow, maxCol,globalIFromOtherThread+1,
                    globalJFromOtherThread)){
            res.addElement(field[0][globalJFromOtherThread-1]);
            res.addElement(field[0][globalJFromOtherThread]);
            res.addElement(field[0][globalJFromOtherThread+1]);
        }
        /* right case */
        else if (isRightSideButNotCorner(maxRow, maxCol,globalIFromOtherThread,
                    globalJFromOtherThread-1)){
            res.addElement(field[globalIFromOtherThread-1][maxCol]);
            res.addElement(field[globalIFromOtherThread][maxCol]);
            res.addElement(field[globalIFromOtherThread+1][maxCol]);
        }
        /* down case */
        else if (isDownSideButNotCorner(maxRow, maxCol,globalIFromOtherThread-1,
                    globalJFromOtherThread)){
            res.addElement(field[maxRow][globalJFromOtherThread-1]);
            res.addElement(field[maxRow][globalJFromOtherThread]);
            res.addElement(field[maxRow][globalJFromOtherThread+1]);
        }
        /* left case */
        else if (isLeftSideButNotCorner(maxRow, maxCol,globalIFromOtherThread,
                    globalJFromOtherThread+1)){
            res.addElement(field[globalIFromOtherThread-1][maxCol]);
            res.addElement(field[globalIFromOtherThread][maxCol]);
            res.addElement(field[globalIFromOtherThread+1][maxCol]);
        }
        return res;
    }

    private void writeResult() {
         
        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        for (int i = 0 ; i <= maxRow ; i++) {
            for (int j = 0 ; j <= maxCol ; j++) {
                result[1][i][j] = field[i][j].getCurrentCopy().isAlive();
                result[0][i][j] = field[i][j].getPreviousCopy().isAlive();
            }
        }
    }

}





