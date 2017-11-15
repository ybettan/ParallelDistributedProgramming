package tests;

import ex1.Cell3D;
import ex1.Cell;


public class Cell3DTest {

    public static void main(String args[]) {

        System.out.print("Cell3DTest...");

        /* constructor & getCurrentCopy test  */
        Cell3D c1 = new Cell3D(true, 8, 3, 3);
        /* check that the exception works */
        //c1.getPreviousCopy();
        test(c1.getCurrentCopy().isAlive() == true);
        test(c1.getCurrentCopy().wasUpdated() == false);
        test(c1.getCurrentCopy().getGen() == 0);
        test(c1.getCurrentCopy().getGlobalI() == 3);
        test(c1.getCurrentCopy().getGlobalJ() == 3);

        /* addNeighbor(), wasUpdated(), getPreviousCopy() and updateFloor() test */
        Cell3D c2 = new Cell3D(false, 4, 3, 3);
        Cell liveCell = new Cell(true, 4, 3, 3);
        Cell deadCell = new Cell(false, 4, 3, 3);
        c2.addNeighbor(liveCell);
        c2.addNeighbor(liveCell);
        c2.addNeighbor(liveCell);
        test(c2.getCurrentCopy().isAlive() == false);
        test(c2.getCurrentCopy().wasUpdated() == false);
        test(c2.getCurrentCopy().getGen() == 0);
        c2.addNeighbor(deadCell);
        test(c2.getCurrentCopy().isAlive() == true);
        test(c2.wasUpdated() == true);
        test(c2.wasUpdated() == false);
        test(c2.getCurrentCopy().getGen() == 1);
        c2.updateFloors();
        test(c2.getPreviousCopy().isAlive() == false);
        test(c2.getPreviousCopy().wasUpdated() == false);
        test(c2.getPreviousCopy().getGen() == 0);
        /* check that nextGen has increased to gen == 2 */
        liveCell.increaseGen();
        liveCell.increaseGen();
        /* if gen < 2 we will receive an exception */
        c2.addNeighbor(liveCell);

        System.out.println("[OK]");
    } 

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

}
