package tests;

import ex1.Cell3D;
import ex1.Cell;


public class Cell3DTest {

    public static void main(String args[]) {

        System.out.print("Cell3DTest...");

        /* constructor & getCellByGen() test  */
        Cell3D c1 = new Cell3D(true, 8, 3, 3);
        test(c1.getCellCopyByGen(-1).isAlive() == true);
        test(c1.getCellCopyByGen(-1).getGen() == -1);
        test(c1.getCellCopyByGen(-1).getGlobalI() == 3);
        test(c1.getCellCopyByGen(-1).getGlobalJ() == 3);
        test(c1.getCellCopyByGen(0).isAlive() == true);
        test(c1.getCellCopyByGen(0).getGen() == 0);
        test(c1.getCellCopyByGen(0).getGlobalI() == 3);
        test(c1.getCellCopyByGen(0).getGlobalJ() == 3);

        /* addNeighbor(), wasUpdated(), getPreviousCopy() and updateFloor() test */
        Cell3D c2 = new Cell3D(false, 4, 3, 3);
        Cell liveCell = new Cell(true, 4, 3, 3);
        Cell deadCell = new Cell(false, 4, 3, 3);
        c2.addNeighbor(liveCell);
        c2.addNeighbor(liveCell);
        c2.addNeighbor(liveCell);
        test(c2.getCellCopyByGen(0).isAlive() == false);
        test(c2.getCellCopyByGen(0).getGen() == 0);
        c2.addNeighbor(deadCell);
        test(c2.getCellCopyByGen(1).isAlive() == true);
        test(c2.getCellCopyByGen(1).getGen() == 1);
        test(c2.getCellCopyByGen(0).isAlive() == false);
        test(c2.getCellCopyByGen(0).getGen() == 0);

        /* getGlobalX() and getGenOfCurrent() test */
        Cell3D c3 = new Cell3D(false, 4, 3, 2);
        test(c3.getGenOfCurrent() == 0);
        test(c3.getGlobalI() == 3);
        test(c3.getGlobalJ() == 2);
        c3.addNeighbor(liveCell);
        c3.addNeighbor(liveCell);
        c3.addNeighbor(liveCell);
        c3.addNeighbor(deadCell);
        test(c3.getGenOfCurrent() == 1);
        test(c3.getGlobalI() == 3);
        test(c3.getGlobalJ() == 2);


        System.out.println("[OK]");
    } 

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

}
