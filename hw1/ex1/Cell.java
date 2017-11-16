package ex1;




public class Cell {

    private boolean isAlive;
    private int gen;
    private int checkedNeighbors;
    private int aliveNeighbors;
    private boolean wasUpdated;
    // not all cells have 8 neighbors
    private final int MAX_NEIGHBORS;        
    private final int GLOBAL_I, GLOBAL_J;

    /*
    C'tor
     */
    public Cell(boolean isAlive, int maxNeighbors, int globalI, int globalJ) {
        this.isAlive = isAlive;
        gen = 0;
        checkedNeighbors = 0;
        aliveNeighbors = 0;
        wasUpdated = false;
        this.MAX_NEIGHBORS = maxNeighbors;
        this.GLOBAL_I = globalI;
        this.GLOBAL_J = globalJ;
    }

    /*
    Copy C'tor
     */
    public Cell(Cell ref) {
        this.isAlive = ref.isAlive;
        this.gen = ref.gen;
        this.checkedNeighbors = ref.checkedNeighbors;
        this.aliveNeighbors = ref.aliveNeighbors;
        this.wasUpdated = ref.wasUpdated;
        this.MAX_NEIGHBORS = ref.MAX_NEIGHBORS;
        this.GLOBAL_I = ref.GLOBAL_I;
        this.GLOBAL_J = ref.GLOBAL_J;
    }

    /*
    Use to increase the Cell's Generation.
     */
    public void increaseGen() {
        gen++;
    }

    /*
    Use to decrease the Cell's Generation.
     */
    public void decreaseGen() {
        gen--;
    }

    /*
    Check if a Cell is alive.
     */
    public boolean isAlive() {
        return isAlive;
    }

    /*
    get the Cell's current Generation.
     */
    public int getGen() {
        return gen;
    }

    /*
    Get the row of the Cell in the original board.
     */
    public int getGlobalI() {
        return GLOBAL_I;
    }

    /*
    Get the column of the Cell in the original board.
     */
    public int getGlobalJ() {
        return GLOBAL_J;
    }

    /*
    Used when a Cell advances, the Cell will need to recieve data from another Cell.
     */
    public void takeDataFrom(Cell c) {
        checkedNeighbors = c.checkedNeighbors;
        aliveNeighbors = c.aliveNeighbors;
    }

    /*
    Given a neighbor cell, we will use it for calculating this Cell.
     */
    public void addNeighbor(Cell c) {
        //FIXME: for debugging - remove when done
        if (gen != c.gen)
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());
        checkedNeighbors++;
        if (c.isAlive()) {
            aliveNeighbors++;
        }
    }

    /* 
     return whether a cell is ready to move to next generation
     */
    public boolean needUpdate() {
        return (checkedNeighbors == MAX_NEIGHBORS);
    }

    /*
    Once a Cell was updated by all his neighbors, this method will move him to 
    the next generation.
     */
    public  void moveToNextGen() {
        // FIXME: just for debugging - remove when finish
        if (aliveNeighbors > checkedNeighbors) 
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());
        // update current version
        if (aliveNeighbors == 3 || (aliveNeighbors == 2 && isAlive) ) 
            isAlive = true;
        else 
            isAlive = false;
        gen++;
        checkedNeighbors = 0;
        aliveNeighbors = 0;
    }

}
