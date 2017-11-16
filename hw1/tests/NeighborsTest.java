package tests;

import ex1.Neighbors;
import ex1.Field;



public class NeighborsTest {

    public static void main(String args[]) {
        boolean res = true;
        System.out.print("NeighborsTest...");

        Field up=null, upRight=null, right=null, downRight=null, down=null, downLeft=null, left=null,
                upLeft=null;
        Neighbors n = new Neighbors(up, upRight, right, downRight,
                down, downLeft,left,upLeft);

        assert (n.getDown() == null );


        System.out.println("[OK]");
    }
}








