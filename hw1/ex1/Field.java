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
                boolean b = field[i][j].getCellByGen(generation).isAlive();
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
                } else if (isSideButNotCorner(numOfRows-1,
                        numOfCols-1, i, j)) {
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
                    sendToNeighbors(field[row][col].getCellByGen(0),row,col);
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
     * recursiveAddNeighbors rece
     */
    private void recursiveAddNeighbors(Cell c) {

        Vector<Cell3D> dependencies = getInerDependecies(c);
        for (Cell3D dep : dependencies) {
            int currGen = dep.getCurrentGeneration();
            dep.addNeighbor(c);
            if (currGen+1 == dep.getCurrentGeneration()) {
                if (currGen+1 == generations) {
                    numOfDoneCells++;
                }
                int minX = field[0][0].getGlobalJ();
                int minY = field[0][0].getGlobalI();
                Cell send = new Cell(dep.getCurrentGen());
                sendToNeighbors(send,send.getGlobalI()-minY,
                        send.getGlobalJ()-minX);
                recursiveAddNeighbors(dep.getCurrentCopy());
            }
        }

    }
    /* find all the Cell3D that need the given Cell in order to continue
     * computing their generations.
     */
    private Vector<Cell3D> getInerDependecies(Cell c) {

        Cell3D minLimit = field[0][0];
        Cell3D maxLimit = field[field.length-1][field[0].length-1];
        int minX = minLimit.getGlobalX();
        int minY = minLimit.getGlobalY();
        int y = c.getGlobalI() - minY;
        int x = c.getGlobalJ() - minX;
        int maxCol = field[0].length;
        int maxRow = field.length;
        Vector<Cell3D> res = new Vector<>();
        if ((y < -1) || (x < -1) || (y > maxRow) || (x > maxCol))
            System.err.println("ERROR getInerDependecies: bad dimensions for dependencies ");

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
                result[1][i][j] = field[i][j].getCellByGen(generations).isAlive();
                result[0][i][j] = field[i][j].getCellByGen(generations-1).isAlive();
            }
        }
    }

}
