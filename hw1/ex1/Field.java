package ex1;

import java.util.ArrayList;


public class Field implements Runnable {

    private Cell3D[][] field;
    private SyncedQueue queue;
    private int numOfDoneCells;
    private int generations;
    private Neighbors neighbors; 
    private final int CORNER_MAX_NEIGHBORS = 3;
    private final int SIDE_MAX_NEIGHBORS = 5;
    private final int INTERNAL_MAX_NEIGHBORS = 8;
    

    public Field(boolean[][] initalField, int minI, int maxI, int minJ, 
            int maxJ, int generations) {
        this.field = createPartialCopy(initalField, minI, maxI, minJ, maxJ);
        queue = new SyncedQueue();
        numOfDoneCells = 0;
        this.generations = generations;
    }
    
    public void setNeighbors(Neighbors neighbors) {
        this.neighbors = neighbors;
    }

    public SyncedQueue getQueue() {
        return queue;
    }

    @Override public void run() {
        autonomusPart();        
        communicationPart();        
    } 

    /* for a IxJ field, autonomus calculation can be done on (I-2)x(J-2) field */
    private void autonomusPart() {
        autonumusCornerPart();
        autonumusSidePart();
        autonumusInternalPart();
    }

    private void communicationPart() {

        int maxCol = field[0].length-1;
        int maxRow = field.length-1;

        /* while not all cells have arrived to required generation */
        while (numOfDoneCells < field.length * field[0].length) {
           Cell cellFromOtherThread = queue.dequeue();

           /* find all the Cell3D that needs that cellFromOtherThread */ 
           ArrayList<Cell3D> destCells3D = new ArrayList<>();
           int globalIFromOtherThread = cellFromOtherThread.getGlobalI();
           int globalJFromOtherThread = cellFromOtherThread.getGlobalJ();
           
           /* cellFromOtherThread come from up neighbor thread */
           if (globalIFromOtherThread < field[0][0].getCurrentCopy().getGlobalI() &&
               globalJFromOtherThread) {
               
           }


           recursiveAddNeighbors(destCells3D, cellFromOtherThread);
        }
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

    private void recursiveAddNeighbors(ArrayList<Cell3D> destCells3D, Cell c ) {
         while (destCells3D.size() > 0) {
             Cell3D dest = destCells3D.get(0);
             destCells3D.remove(0);
         }
    }

    private Cell3D[][] createPartialCopy(boolean[][] initalField, int minI, int maxI, 
            int minJ, int maxJ) {
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
                if (isCorner(initalField, i, j)) {
                    maxNeighbors = CORNER_MAX_NEIGHBORS;
                } else if (isSide(initalField, i, j)) {
                    maxNeighbors = SIDE_MAX_NEIGHBORS;
                } else {
                    maxNeighbors = INTERNAL_MAX_NEIGHBORS;
                } 
                res[i-minI][j-minJ] = new Cell3D(isAlive, maxNeighbors, i, j);
            }
        }
        return res;
    }

    private boolean isCorner(boolean[][] initalField, int i, int j) {
        boolean res = false;
        if (i == 0 && j == 0) {
            res = true;
        }
        if (i == 0 && j == initalField[0].length - 1) {
            res = true;
        }
        if (i == initalField.length - 1 && j == 0) {
            res = true;
        }
        if (i == initalField.length - 1 && j == initalField[0].length - 1) {
            res = true;
        }
        return res;
    }

    private boolean isSide(boolean[][] initalField, int i, int j) {
        boolean res = false;
        if ( (i == 0 || i == initalField.length) && !isCorner(initalField,i,j)) {
            res = true;
        }
        if ( (j == 0 || i == initalField[0].length) && !isCorner(initalField,i,j)) {
            res = true;
        }
        return res;
    }
}


