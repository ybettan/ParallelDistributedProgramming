package ex1;

import java.util.Vector;

public class Field implements Runnable {

    private Cell3D[][] field;
    private SyncedQueue queue;
    private int numOfDoneCells;
    private int generations;
    private Neighbors neighbors;
    private boolean[][][] result; 
    private final int SIDE_MAX_NEIGHBORS_1D = 1;
    private final int INTERNAL_MAX_NEIGHBORS_1D = 2;
    private final int CORNER_MAX_NEIGHBORS_2D = 3;
    private final int SIDE_MAX_NEIGHBORS_2D = 5;
    private final int INTERNAL_MAX_NEIGHBORS_2D = 8;
    private final int ERROR_CODE = -1;


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
        autonomousPart();
        communicationPart();
        writeResult();
    }

//-----------------------------------------------------------------------------
//                      FIXME:DEBUGGING - remove
//-----------------------------------------------------------------------------

    public  void printField(int generation) {
        for (int i=0 ; i<field.length ; i++) {
            for (int j=0 ; j<field[0].length ; j++) {
                boolean b = field[i][j].getCellCopyByGen(generation).isAlive();
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
    /*
     * Used to make a copy of the initial field as received. Each field will
     * copy only the relevant parts to him, and create 3D Cells from it.
     * While Creating the 3D Cell, the global location is passed, hence the Cell
     * knows if it is waiting for (3 | 5 | 8) neighbors.
     */
    public Cell3D[][] createPartialCopy(boolean[][] initalField, int minI,
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
                int maxNeighbors = getMaxNeighbors(initalField, minI+i, minJ+j);
                res[i-minI][j-minJ] = new Cell3D(isAlive, maxNeighbors, i, j);
            }
        }
        return res;
    }

    public int getMaxNeighbors(boolean[][] initalField, int globalI, int globalJ) {

        int maxRow = initalField.length-1;
        int maxCol = initalField[0].length-1;

        /* return -1 when go out of the array */
        if (globalI < 0 || globalI > maxRow || globalJ < 0 || globalJ > maxCol)
            return ERROR_CODE;

        /* the field is 1D */
        if (maxRow == 0) {
            if (globalJ == 0 || globalJ == maxCol)
                return SIDE_MAX_NEIGHBORS_1D;
            else
                return INTERNAL_MAX_NEIGHBORS_1D;
        }
        else if (maxCol == 0) {
            if (globalI == 0 || globalI == maxRow)
                return SIDE_MAX_NEIGHBORS_1D;
            else
                return INTERNAL_MAX_NEIGHBORS_1D;
        }

        /* the field is 2D */
        else if ( (globalI == 0 || globalI == maxRow) &&
                (globalJ == 0 || globalJ == maxCol)      )
            return CORNER_MAX_NEIGHBORS_2D;

        else if (globalI==0||globalI==maxRow||globalJ==0||globalJ==maxCol)
            return SIDE_MAX_NEIGHBORS_2D;

        else
            return INTERNAL_MAX_NEIGHBORS_2D;

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

    public void autonomousPart() {
        /* send the Cells that are located on the margins of the board to the
        to the threads that will wait or are waiting for it. */
        sendMargins();
        /* build a pyramid of generations by data that the thread can compute on
        its own. */
        buildPyramid(0, field.length, 0, field[0].length, 0);
    }

    /*
     * send the margin cells to the neighbors.
     */
    private void sendMargins() {
        for (int row=0; row < field.length; row++) {
            for (int col=0; col < field[0].length; col++){
                if ((row == 0) || (row==field.length-1) || (col==0) || 
                        (col==field[0].length-1))
                    sendToNeighbors(field[row][col].getCellCopyByGen(0),row,col);
            }
        }
    }
    /*
     * build a recursive pyramid of generations. each time the function will 
     * compute the generation for the given base
     * which the dimensions limited by: rows start at min row and end at max
     *                                  row.
     *                                  columns start at min col and end at max
     *                                  col.
     * next the the function will calculate the new dimensions for the next base
     * and call it self again.
     * each time the generation that is passed should increase by 1.
     * the recursion will stop if:
     *          1) the base is to small to compute.
     *          2) if the max generation was calculated.
     *          3) if no cell advanced a generation during
     */
    private void buildPyramid(int minRow, int maxRow, int minCol, int maxCol, 
            int currentGeneration) {
        // stopping conditions.
        if (currentGeneration == generations) {
            numOfDoneCells = (maxCol-minCol) * (maxRow-minRow);
            return;
        }
        if ((minRow>=maxRow-1) && (minCol>=maxCol-1))
            return;
        // limits for next iteration.
        int nextMinRow = -1, nextMaxRow = -1, nextMinCol = -1, nextMaxCol = -1;

        for (int row=minRow; row < maxRow; row++){
            for (int col=minCol; col < maxCol; col++) {
                // look up:
                if (row > minRow) {
                    field[row][col].addNeighbor(
                            field[row-1][col].getCellCopyByGen(currentGeneration));
                    // look up and left
                    if (col>minCol)
                        field[row][col].addNeighbor(
                                field[row-1][col-1].getCellCopyByGen(currentGeneration));
                    // look up and right
                    if (col < maxCol-1)
                        field[row][col].addNeighbor(
                                field[row-1][col+1].getCellCopyByGen(currentGeneration));
                }
                // look down
                if (row < maxRow-1) {
                    field[row][col].addNeighbor(
                            field[row+1][col].getCellCopyByGen(currentGeneration));
                    // look down and right
                    if (col < maxCol-1)
                        field[row][col].addNeighbor(
                                field[row+1][col+1].getCellCopyByGen(currentGeneration));
                    // look down and left
                    if (col>minCol)
                        field[row][col].addNeighbor(
                                field[row+1][col-1].getCellCopyByGen(currentGeneration));
                }
                // look left
                if (col>minCol)
                    field[row][col].addNeighbor(
                            field[row][col-1].getCellCopyByGen(currentGeneration));
                // look right
                if (col < maxCol-1)
                    field[row][col].addNeighbor(
                            field[row][col+1].getCellCopyByGen(currentGeneration));
                // find the first cell that was updated. he is the top 
                // left corner of the pyramid above.
                if ((nextMinCol == -1) && 
                        (field[row][col].getCellCopyByGen(currentGeneration+1) != null)) {
                    nextMinCol = col;
                    nextMinRow = row;
                }
                // find the last cell that was updated. 
                // he is the bottom left corner of the pyramid above.
                if (field[row][col].getCellCopyByGen(currentGeneration+1) != null) {
                    nextMaxCol = col;
                    nextMaxRow = row;
                }
            }
        }
        buildPyramid(nextMinRow, nextMaxRow+1, nextMinCol,
                nextMaxCol+1,currentGeneration+1);
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

    private void sendToNeighbors(Cell c, int localRow, int localCol) {
        Field neighbor;
        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        /* up neighbor case*/
        neighbor = neighbors.getUp();
        if (neighbor != null && localRow == 0) {
           neighbor.getQueue().enqueue(c);
        }
        /* up right neighbor case*/
        neighbor = neighbors.getUpRight();
        if (neighbor != null && localRow == 0 && localCol == maxCol) {
           neighbor.getQueue().enqueue(c);
        }
        /* right neighbor case*/
        neighbor = neighbors.getRight();
        if (neighbor != null &&  localCol == maxCol) {
           neighbor.getQueue().enqueue(c);
        }
        /* down right neighbor case*/
        neighbor = neighbors.getDownRight();
        if (neighbor != null &&  localRow == maxRow && localCol == maxCol) {
           neighbor.getQueue().enqueue(c);
        }
        /* down neighbor case*/
        neighbor = neighbors.getDown();
        if (neighbor != null &&  localRow == maxRow) {
           neighbor.getQueue().enqueue(c);
        }
        /* down left neighbor case*/
        neighbor = neighbors.getDownLeft();
        if (neighbor != null &&  localRow == maxRow && localCol == 0) {
           neighbor.getQueue().enqueue(c);
        }
        /* left neighbor case*/
        neighbor = neighbors.getLeft();
        if (neighbor != null && localCol == 0) {
           neighbor.getQueue().enqueue(c); 
        }
        /* up left neighbor case*/
        neighbor = neighbors.getUpLeft();
        if (neighbor != null && localRow == 0 && localCol == 0) {
           neighbor.getQueue().enqueue(c);
        }
    }

    /*
     * recursiveAddNeighbors receive a cell and does the following steps:
     * 1) finds all the Cell3D that are neighbors and belong to the same thread.
     * 2) updates the neighbors of the cell.
     * 3) if a neighbor was updated.
     *      3.1) if his generation is the target generation, no body needs the
     *           cell, continue.
     *      3.2) if the neighbor was updated try sending him to the neighbor
     *           threads. (if his not a margin he wont be sent).
     *      3.3) call recursiveAddNeighbors with the updated cell so cells
     *           around him will also be updated.
     */
    private void recursiveAddNeighbors(Cell c) {
        // step 1. find dependencies.
        Vector<Cell3D> dependencies = getInerDependecies(c);
        for (Cell3D dep : dependencies) {
            // get the current generation, use to check if cell advanced.
            int currGen = dep.getGenOfCurrent();
            // step 2. update the cell.
            dep.addNeighbor(c);
            // step 3. if the cell was updated.
            if (currGen+1 == dep.getGenOfCurrent()) {
                // step 3.1. if we got to the final generation
                if (currGen+1 == generations) {
                    numOfDoneCells++;
                    continue;
                }
                int minX = field[0][0].getGlobalJ();
                int minY = field[0][0].getGlobalI();
                Cell send = new Cell(dep.getCellCopyByGen(currGen+1));
                // step 3.2. try to send to neighbor threads.
                sendToNeighbors(send,send.getGlobalI()-minY,
                        send.getGlobalJ()-minX);
                // step 3.3. update neighbor cells.
                recursiveAddNeighbors(send);
            }
        }

    }
    /*
     * find all the Cell3D that need the given Cell in order to continue
     * computing their generations.
     */
    private Vector<Cell3D> getInerDependecies(Cell c) {

        Cell3D minLimit = field[0][0];
        // global top left corner of the threads field.
        int minX = minLimit.getGlobalJ();
        int minY = minLimit.getGlobalI();
        // cells location in the global board, in the threads point of view.
        int y = c.getGlobalI() - minY;
        int x = c.getGlobalJ() - minX;
        int maxCol = field[0].length;
        int maxRow = field.length;

        Vector<Cell3D> res = new Vector<>();
        //x and y are relevant only if x is in [-1..maxCol] y is in [-1..maxRow]
        if ((y < -1) || (x < -1) || (y > maxRow) || (x > maxCol)) {
            System.err.println("ERROR getInerDependecies: bad dimensions for dependencies ");
            return res;
        }

        // look up
        if (y > 0) {
            // look directly up
            if ((x >= 0) && (x < maxCol))
                res.addElement(field[y-1][x]);
            // look up and left
            if ((x > 0) && (x < maxCol))
                res.addElement(field[y-1][x-1]);
            // look up and right
            if ((x >=0) && (x < maxCol-1))
                res.addElement(field[y-1][x+1]);
        }
        // look down
        if (y < maxRow){
            // look directly down
            if ((x >= 0) && (x < maxCol))
                res.addElement(field[y+1][x]);

            if ((x > 0) && (x < maxCol)) {
                // look down and left
                res.addElement(field[y + 1][x - 1]);
                // look directly left
                if (y >= 0)
                    res.addElement(field[y][x-1]);
            }
            if ((x >=0) && (x < maxCol-1)) {
                // look down and right
                res.addElement(field[y + 1][x + 1]);
                // look directly right
                if (y >= 0)
                    res.addElement(field[y][x+1]);
            }
        }
        return res;
    }

    public void writeResult() {
         
        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        for (int i = 0 ; i <= maxRow ; i++) {
            for (int j = 0 ; j <= maxCol ; j++) {
                result[1][i][j] = field[i][j].getCellCopyByGen(generations).isAlive();
                result[0][i][j] = field[i][j].getCellCopyByGen(generations-1).isAlive();
            }
        }
    }
}
