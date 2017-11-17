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
        previousGen = new Cell(currentGen);
        nextGen = new Cell(currentGen);
        nextGen.increaseGen();
        previousGen.decreaseGen();
    }

    /*
    Get an update from a neighbor Cell.
    If the update is not legale the Cell may throw an exeption.
    do nothing if input cell is null
     */
    public void addNeighbor(Cell c) {
        if (c == null) return;

        /* if this.getGet() == g than c.getGen() == {g, g+1} */
        if (c.getGen() == currentGen.getGen()) {
            currentGen.addNeighbor(c);
            if (currentGen.needUpdate()) {
                previousGen = new Cell(currentGen);
                currentGen.moveToNextGen();
                currentGen.takeDataFrom(nextGen);
                nextGen.moveToNextGen();
            }
        } else {
            /* must be g+1, an exception will be throw at Cell level if not */
            nextGen.addNeighbor(c);
        }
    }

    /*
     * return a COPY to the relevant cell by its generation
     * can return only currentGen and previousGen
     * don't have access to nextGen
     */
    public Cell getCellCopyByGen(int gen) {
        if (previousGen.getGen() == gen) {
            return new Cell(previousGen);
        }
        else if (currentGen.getGen() == gen) {
            return new Cell(currentGen);
        }
        else {
            return null;
        }
        
    }

    /*
     * return the global row index of the Cell3D in the original field
     */
    public int getGlobalI() {
        return currentGen.getGlobalI();
    }

    /*
     * return the global colon index of the Cell3D in the original field
     */
    public int getGlobalJ() {
        return currentGen.getGlobalJ();
    }

}
