package ex1;

/*
 Cell3d is used to monitor a Cell through out all the game.
 it monitors the current Generation and 1 Generation above and below.
 */
public class Cell3D {

    private Cell nextGen;
    private Cell currentGen;
    private Cell previousGen;

    /*
    C'tor
    isAlive - the initial state of the Cell.
    maxNeighbors - the amount of neighbors the Cell has, a Cell that is located at:
                    corner - has 3 neighbors.
                    side - has 5 nighbors.
                    internal\regular - has 8 neighbors.
    globalI - the original row of the Cell in the original board.
    globalJ - the original column of the Cell in the original board.
     */
    public Cell3D(boolean isAlive, int maxNeighbors, int globalI, int globalJ) {
        currentGen = new Cell(isAlive, maxNeighbors, globalI, globalJ);
        nextGen = new Cell(currentGen);
        nextGen.increaseGen();
    }

    /*
    Returns a ~new~ copy of the Cell in the current Generation.
     */
    public Cell getCurrentCopy() {
        return new Cell(currentGen);
    }

    /* FIXME: can i assume that they won't give input gen < 2 ? */
    public Cell getPreviousCopy() {
        if (previousGen == null)
            System.err.println("previousGen wasn't initiallize yet: line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());
        return new Cell(previousGen);
    }

    /*
    Check if a Cell was updated.
    Once the function returns true on Generation x it will not return true again!
     */
    public boolean wasUpdated() {
       return currentGen.wasUpdated(); 
    }

    /*
    Get an update from a neighbor Cell.
    If the update is not legale the Cell may throw an exeption.
     */
    public void addNeighbor(Cell c) {
        /* if this.getGet() == g than c.getGen() == {g, g+1} */
        if (c.getGen() == currentGen.getGen()) {
            if (currentGen.needOneMoreNeighbor()) {
                previousGen = new Cell(currentGen);
            }
            currentGen.addNeighbor(c);
        } else {
            /* must be g+1, an exception will be throw at Cell level if not */
            nextGen.addNeighbor(c);
        }
    }

    /*
    Once a Cell was updated we want to push it foward.
    assumes that currentGen just was updated
    */
    public  void updateFloors() {
        currentGen.takeDataFrom(nextGen);
        nextGen.increaseGen();
    }

}
