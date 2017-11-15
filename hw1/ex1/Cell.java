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
    
    public void increaseGen() {
        gen++;
    }

    public boolean needOneMoreNeighbor() {
        return checkedNeighbors == MAX_NEIGHBORS - 1;
    }

    public boolean isAlive() {
        return isAlive;
    }

    public boolean wasUpdated() {
        boolean tmp = wasUpdated;
        wasUpdated = false;
        return tmp;
    }

    public int getGen() {
        return gen;
    }

    public int getGlobalI() {
        return GLOBAL_I;
    }

    public int getGlobalJ() {
        return GLOBAL_J;
    }

    public void takeDataFrom(Cell c) {
        checkedNeighbors = c.checkedNeighbors;
        aliveNeighbors = c.aliveNeighbors;
    }

    // given a neighbor cell we can add it to the calculation for our cell
    public void addNeighbor(Cell c) {
        //FIXME: for debugging - remove when done
        if (gen != c.gen)
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());
        checkedNeighbors++;
        if (c.isAlive()) {
            aliveNeighbors++;
        }
        // if we handlend all neighbors we can update the cell to next gen
        if (checkedNeighbors == MAX_NEIGHBORS) {
            moveToNextGen();
        }
    }
    
    private void moveToNextGen() {
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
        wasUpdated = true;
    }

}
