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
//                              private methods
//-----------------------------------------------------------------------------

    private Cell3D[][] createPartialCopy(boolean[][] initalField, int minI, 
            int maxI, int minJ, int maxJ) {

        int numOfRows = maxJ - minJ + 1;
        int numOfCols = maxI - minI + 1;
        int buttomBoundry = initalField.length;
        int rightBoundry = initalField[0].length;
        Cell3D[][] res = new Cell3D[numOfRows][];
        for (int i = minI ; i <= maxI ; i++) {
            res[i] = new Cell3D[numOfCols];
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
        autonumusCornerPart();
        autonumusSidePart();
        autonumusInternalPart();
    }

    private void autonumusCornerPart() {

        int maxCol = field[0].length-1;
        int maxRow = field.length-1;


        /* top left corner */
        sendToNeighbors(field[0][0].getCurrentCopy(), 0, 0);
        field[0][0].addNeighbor(field[0][1].getCurrentCopy());                 
        field[0][0].addNeighbor(field[1][1].getCurrentCopy());                 
        field[0][0].addNeighbor(field[1][0].getCurrentCopy());                 

        /* top right corner */
        sendToNeighbors(field[0][maxCol].getCurrentCopy(), 0, maxCol);
        field[0][maxCol].addNeighbor(field[0][maxCol-1].getCurrentCopy());                 
        field[0][maxCol].addNeighbor(field[1][maxCol-1].getCurrentCopy());                 
        field[0][maxCol].addNeighbor(field[1][maxCol].getCurrentCopy());                 

        /* buttom right corner */
        sendToNeighbors(field[maxRow][maxCol].getCurrentCopy(), maxRow, maxCol);
        field[maxRow][maxCol].addNeighbor(field[maxRow][maxCol-1].getCurrentCopy());
        field[maxRow][maxCol].addNeighbor(field[maxRow-1][maxCol-1].getCurrentCopy());
        field[maxRow][maxCol].addNeighbor(field[maxRow-1][maxCol].getCurrentCopy());

        /* buttom left corner */
        sendToNeighbors(field[maxRow][0].getCurrentCopy(), maxRow, 0);
        field[maxRow][0].addNeighbor(field[maxRow-1][0].getCurrentCopy());                 
        field[maxRow][0].addNeighbor(field[maxRow-1][1].getCurrentCopy());                 
        field[maxRow][0].addNeighbor(field[maxRow][1].getCurrentCopy());                 
    }

    private void autonumusSidePart() {

        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        /* upSide */
        for (int i = 1 ; i < maxCol ; i++) {
            sendToNeighbors(field[0][i].getCurrentCopy(), 0, i);
            field[0][i].addNeighbor(field[0][i-1].getCurrentCopy());
            field[0][i].addNeighbor(field[1][i-1].getCurrentCopy());
            field[0][i].addNeighbor(field[1][i].getCurrentCopy());
            field[0][i].addNeighbor(field[1][i+1].getCurrentCopy());
            field[0][i].addNeighbor(field[0][i+1].getCurrentCopy());
        }

        /* rightSide */
        for (int i = 1 ; i < maxRow ; i++) {
            sendToNeighbors(field[i][maxCol].getCurrentCopy(), i, maxCol);
            field[i][maxCol].addNeighbor(field[i-1][maxCol].getCurrentCopy());
            field[i][maxCol].addNeighbor(field[i-1][maxCol-1].getCurrentCopy());
            field[i][maxCol].addNeighbor(field[i][maxCol-1].getCurrentCopy());
            field[i][maxCol].addNeighbor(field[i+1][maxCol-1].getCurrentCopy());
            field[i][maxCol].addNeighbor(field[i+1][maxCol].getCurrentCopy());
        }

        /* downSide */
        for (int i = 1 ; i < maxCol ; i++) {
            sendToNeighbors(field[maxRow][i].getCurrentCopy(), maxRow, i);
            field[maxRow][i].addNeighbor(field[maxRow][i-1].getCurrentCopy());
            field[maxRow][i].addNeighbor(field[maxRow-1][i-1].getCurrentCopy());
            field[maxRow][i].addNeighbor(field[maxRow-1][i].getCurrentCopy());
            field[maxRow][i].addNeighbor(field[maxRow-1][i+1].getCurrentCopy());
            field[maxRow][i].addNeighbor(field[maxRow][i+1].getCurrentCopy());
        }

        /* leftSide */
        for (int i = 1 ; i < maxRow ; i++) {
            sendToNeighbors(field[i][0].getCurrentCopy(), i, 0);
            field[i][0].addNeighbor(field[i-1][0].getCurrentCopy());
            field[i][0].addNeighbor(field[i-1][1].getCurrentCopy());
            field[i][0].addNeighbor(field[i][1].getCurrentCopy());
            field[i][0].addNeighbor(field[i+1][1].getCurrentCopy());
            field[i][0].addNeighbor(field[i+1][0].getCurrentCopy());
        }

    }
    
    private void autonumusInternalPart() {

        /* now create the piramide */
        int iterations;
        if (field.length < field[0].length) {
            iterations = field.length;
        } else {
            iterations = field[0].length;
        }
        iterations = iterations/2;
        /* each time work on a smaller rektangle */
        for (int k = 1 ; k <= iterations ; k++) {
            for (int i = k ; i < field.length-k ; i++) {
                for (int j = k ; j < field[0].length-k ; j++) {
                    field[i][j].addNeighbor(field[i-1][j].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i-1][j+1].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i][j+1].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i+1][j+1].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i+1][j].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i+1][j-1].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i][j-1].getCurrentCopy());                 
                    field[i][j].addNeighbor(field[i-1][j-1].getCurrentCopy());                 

                    /* when numOfDoneCells reach board size then the thread 
                     * has finish its calculations */
                    if (field[i][j].getCurrentCopy().getGen() == generations) {
                        numOfDoneCells++;
                    }
                    /* this is done only to turn off the flag */
                    field[i][j].wasUpdated();                 
                }
            }
        }
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





