package ex1;

public class Cell3D {

    private Cell nextGen;
    private Cell currentGen;
    private Cell previousGen;

    public Cell3D(boolean isAlive, int maxNeighbors, int globalI, int globalJ) {
        currentGen = new Cell(isAlive, maxNeighbors, globalI, globalJ);
        nextGen = new Cell(currentGen);
        nextGen.increaseGen();
    }

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

    public boolean wasUpdated() {
       return currentGen.wasUpdated(); 
    }

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

    /* assumes that currentGen just was updated */
    public  void updateFloors() {
        currentGen.takeDataFrom(nextGen);
        nextGen.increaseGen();
    }

}
