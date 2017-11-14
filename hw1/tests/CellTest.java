package tests;

import ex1.Cell;


public class CellTest {

    public static void main(String args[]) {

        System.out.print("CellTest...");

        /* constructor test & isAlive() test */
        Cell c1 = new Cell(true, 4);
        Cell c2 = new Cell(false, 200);
        Cell c3 = new Cell(true, 15);
        Cell c4 = new Cell(false, 8);
        Cell c5 = new Cell(true, 4);
        Cell c6 = new Cell(false, 200);
        Cell c7 = new Cell(true, 15);
        Cell c8 = new Cell(false, 8);
        test(c1.isAlive() == true);
        test(c2.isAlive() == false);
        test(c3.isAlive() == true);
        test(c4.isAlive() == false);
        test(c5.isAlive() == true);
        test(c6.isAlive() == false);
        test(c7.isAlive() == true);
        test(c8.isAlive() == false);

        /* wasUpdated(), getGen() & addNeighbor() test */
        Cell deadCell = new Cell(false, 4);
        Cell liveCell = new Cell(true, 4);
        Cell c9 = new Cell(false, 4);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 0);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        test(c9.isAlive() == false);
        test(c9.wasUpdated() == true);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 1);
        /* this and the next copies are to update the inputCell gen */
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(deadCell);
        deadCell = new Cell(c9);
        test(liveCell.isAlive() == true);
        test(deadCell.isAlive() == false);
        test(liveCell.getGen() == 1);
        test(deadCell.getGen() == 1);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        test(c9.isAlive() == false);
        test(c9.wasUpdated() == true);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 2);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(deadCell);
        deadCell = new Cell(c9);
        test(liveCell.isAlive() == true);
        test(deadCell.isAlive() == false);
        test(liveCell.getGen() == 2);
        test(deadCell.getGen() == 2);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        test(c9.isAlive() == false);
        test(c9.wasUpdated() == true);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 3);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(liveCell);
        liveCell.addNeighbor(deadCell);
        deadCell = new Cell(c9);
        test(liveCell.isAlive() == true);
        test(deadCell.isAlive() == false);
        test(liveCell.getGen() == 3);
        test(deadCell.getGen() == 3);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(deadCell);
        test(c9.isAlive() == true);
        test(c9.wasUpdated() == true);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 4);
        deadCell.addNeighbor(deadCell);
        deadCell.addNeighbor(deadCell);
        deadCell.addNeighbor(deadCell);
        deadCell.addNeighbor(deadCell);
        liveCell = new Cell(c9);
        test(liveCell.isAlive() == true);
        test(deadCell.isAlive() == false);
        test(liveCell.getGen() == 4);
        test(deadCell.getGen() == 4);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(deadCell);
        c9.addNeighbor(deadCell);
        test(c9.isAlive() == true);
        test(c9.wasUpdated() == true);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 5);
        deadCell.addNeighbor(deadCell);
        deadCell.addNeighbor(deadCell);
        deadCell.addNeighbor(deadCell);
        deadCell.addNeighbor(deadCell);
        liveCell = new Cell(c9);
        test(liveCell.isAlive() == true);
        test(deadCell.isAlive() == false);
        test(liveCell.getGen() == 5);
        test(deadCell.getGen() == 5);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        c9.addNeighbor(liveCell);
        test(c9.isAlive() == false);
        test(c9.wasUpdated() == true);
        test(c9.wasUpdated() == false);
        test(c9.getGen() == 6);

        System.out.println("[OK]");
    } 

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

}








