package tests;

import ex1.Field;
import ex1.Neighbors;


public class FieldPrivateTest {

    public static void main(String args[]) {

        System.out.print("FieldTest...");

        boolean[][] initalField1 = { {false, false, true, true, false, false},
                                     {false, false, true, true, false, false},
                                     {true, true, false, false, true, true},
                                     {true, true, false, false, true, true},
                                     {false, false, true, true, false, false},
                                     {false, false, true, true, false, false} };

        boolean[][] initalField2 = { {false, false, true, true, false, false} };
                                    
        boolean[][] initalField3 = { {false},
                                     {true},
                                     {false},
                                     {true} };

        boolean[][] initalField4 = { {false, true, true, true, false, false},
                                     {true, false, true, true, false, false},
                                     {true, true, false, true, true, true},
                                     {true, true, true, false, true, true},
                                     {false, false, true, true, false, false},
                                     {false, false, true, true, false, false} };

        
        boolean[][][] resultField1 = new boolean[2][][];
        resultField1[0] = new boolean[6][];
        resultField1[1] = new boolean[6][];
            for (int i=0 ; i<6 ; i++) {
                resultField1[0][i] = new boolean[6];
                resultField1[1][i] = new boolean[6];
            }

        boolean[][][] resultField2 = new boolean[2][][];
        resultField2[0] = new boolean[1][];
        resultField2[1] = new boolean[1][];
        resultField2[0][0] = new boolean[6];
        resultField2[1][0] = new boolean[6];

        boolean[][][] resultField3 = new boolean[2][][];
        resultField3[0] = new boolean[6][];
        resultField3[1] = new boolean[6][];
            for (int i=0 ; i<6 ; i++) {
                resultField3[0][i] = new boolean[1];
                resultField3[1][i] = new boolean[1];
            }

        boolean[][][] resultField4 = new boolean[2][][];
        resultField4[0] = new boolean[6][];
        resultField4[1] = new boolean[6][];
            for (int i=0 ; i<6 ; i++) {
                resultField4[0][i] = new boolean[6];
                resultField4[1][i] = new boolean[6];
            }

        /* constructor and createPartialCopy()  */
        Field f10 = new Field(initalField1, 0, 1, 0, 1, 3, resultField1);
        Field f11 = new Field(initalField1, 0, 1, 2, 3, 3, resultField1);
        Field f12 = new Field(initalField1, 0, 1, 4, 5, 3, resultField1);
        Field f13 = new Field(initalField1, 2, 3, 0, 1, 3, resultField1);
        Field f14 = new Field(initalField1, 2, 3, 2, 3, 3, resultField1);
        Field f15 = new Field(initalField1, 2, 3, 4, 5, 3, resultField1);
        Field f16 = new Field(initalField1, 4, 5, 0, 1, 3, resultField1);
        Field f17 = new Field(initalField1, 4, 5, 2, 3, 3, resultField1);
        Field f18 = new Field(initalField1, 4, 5, 4, 5, 3, resultField1);

        //System.out.println("initalField1");
        //f10.printField();
        //f11.printField();
        //f12.printField();
        //f13.printField();
        //f14.printField();
        //f15.printField();
        //f16.printField();
        //f17.printField();
        //f18.printField();

        Field f20 = new Field(initalField2, 0, 0, 0, 1, 8, resultField2);
        Field f21 = new Field(initalField2, 0, 0, 2, 3, 8, resultField2);
        Field f22 = new Field(initalField2, 0, 0, 4, 5, 8, resultField2);

        ////System.out.println("initalField2");
        ////f20.printField();
        ////f21.printField();
        ////f22.printField();

        Field f30 = new Field(initalField3, 0, 0, 0, 0, 4, resultField3);
        Field f31 = new Field(initalField3, 1, 1, 0, 0, 4, resultField3);
        Field f32 = new Field(initalField3, 2, 2, 0, 0, 4, resultField3);
        Field f33 = new Field(initalField3, 3, 3, 0, 0, 4, resultField3);

        //System.out.println("initalField3");
        //f30.printField();
        //f31.printField();
        //f32.printField();
        //f33.printField();
        
        Field f40 = new Field(initalField4, 0, 1, 0, 1, 1, resultField4);
        Field f41 = new Field(initalField4, 0, 1, 2, 3, 1, resultField4);
        Field f42 = new Field(initalField4, 0, 1, 4, 5, 1, resultField4);
        Field f43 = new Field(initalField4, 2, 3, 0, 1, 1, resultField4);
        Field f44 = new Field(initalField4, 2, 3, 2, 3, 1, resultField4);
        Field f45 = new Field(initalField4, 2, 3, 4, 5, 1, resultField4);
        Field f46 = new Field(initalField4, 4, 5, 0, 1, 1, resultField4);
        Field f47 = new Field(initalField4, 4, 5, 2, 3, 1, resultField4);
        Field f48 = new Field(initalField4, 4, 5, 4, 5, 1, resultField4);

        /* test getMaxNeighbors() */
        test(f10.getMaxNeighbors(initalField1, 0, 0) == 3);
        test(f10.getMaxNeighbors(initalField1, 0, 5) == 3);
        test(f10.getMaxNeighbors(initalField1, 5, 0) == 3);
        test(f10.getMaxNeighbors(initalField1, 5, 5) == 3);
        test(f10.getMaxNeighbors(initalField1, 0, 3) == 5);
        test(f10.getMaxNeighbors(initalField1, 5, 3) == 5);
        test(f10.getMaxNeighbors(initalField1, 3, 0) == 5);
        test(f10.getMaxNeighbors(initalField1, 3, 5) == 5);
        test(f10.getMaxNeighbors(initalField1, 3, 3) == 8);
        test(f10.getMaxNeighbors(initalField1, 2, 4) == 8);
        test(f10.getMaxNeighbors(initalField2, 8, 8) == -1);
        
        test(f10.getMaxNeighbors(initalField2, 0, 0) == 1);
        test(f10.getMaxNeighbors(initalField2, 0, 1) == 2);
        test(f10.getMaxNeighbors(initalField2, 0, 2) == 2);
        test(f10.getMaxNeighbors(initalField2, 0, 3) == 2);
        test(f10.getMaxNeighbors(initalField2, 0, 4) == 2);
        test(f10.getMaxNeighbors(initalField2, 0, 5) == 1);
        test(f10.getMaxNeighbors(initalField2, 1, 3) == -1);
        
        test(f10.getMaxNeighbors(initalField3, 0, 0) == 1);
        test(f10.getMaxNeighbors(initalField3, 1, 0) == 2);
        test(f10.getMaxNeighbors(initalField3, 2, 0) == 2);
        test(f10.getMaxNeighbors(initalField3, 3, 0) == 1);
        test(f10.getMaxNeighbors(initalField3, 3, 3) == -1);

        /* send Margins() test */
        f40.setNeighbors(new Neighbors(null, null, f41, f44, f43, null, null, null));
        f41.setNeighbors(new Neighbors(null, null, f42, f45, f44, f43, f40, null));
        f42.setNeighbors(new Neighbors(null, null, null, null, f45, f44, f41, null));
        f43.setNeighbors(new Neighbors(f40, f41, f44, f47, f46, null, null, null));
        f44.setNeighbors(new Neighbors(f41, f42, f45, f48, f47, f46, f43, f40));
        f45.setNeighbors(new Neighbors(f42, null, null, null, f48, f47, f44, f41));
        f46.setNeighbors(new Neighbors(f43, f44, f47, null, null, null, null, null));
        f47.setNeighbors(new Neighbors(f44, f45, f48, null, null, null, f46, f43));
        f48.setNeighbors(new Neighbors(f45, null, null, null, null, null, f47, f44));
        
        f40.sendMargins();
        test(f41.areEqualCells(f41.getQueue().dequeue(), f40.getCellCopyByGen(0, 1, 0)));
        test(f41.areEqualCells(f41.getQueue().dequeue(), f40.getCellCopyByGen(1, 1, 0)));
        test(f43.areEqualCells(f43.getQueue().dequeue(), f40.getCellCopyByGen(1, 0, 0)));
        test(f43.areEqualCells(f43.getQueue().dequeue(), f40.getCellCopyByGen(1, 1, 0)));
        test(f43.areEqualCells(f44.getQueue().dequeue(), f40.getCellCopyByGen(1, 1, 0)));

        f44.sendMargins();
        test(f44.areEqualCells(f40.getQueue().dequeue(), f44.getCellCopyByGen(0, 0, 0)));
        test(f44.areEqualCells(f41.getQueue().dequeue(), f44.getCellCopyByGen(0, 0, 0)));
        test(f44.areEqualCells(f43.getQueue().dequeue(), f44.getCellCopyByGen(0, 0, 0)));

        test(f44.areEqualCells(f41.getQueue().dequeue(), f44.getCellCopyByGen(0, 1, 0)));
        test(f44.areEqualCells(f42.getQueue().dequeue(), f44.getCellCopyByGen(0, 1, 0)));
        test(f44.areEqualCells(f45.getQueue().dequeue(), f44.getCellCopyByGen(0, 1, 0)));

        test(f44.areEqualCells(f43.getQueue().dequeue(), f44.getCellCopyByGen(1, 0, 0)));
        test(f44.areEqualCells(f46.getQueue().dequeue(), f44.getCellCopyByGen(1, 0, 0)));
        test(f44.areEqualCells(f47.getQueue().dequeue(), f44.getCellCopyByGen(1, 0, 0)));

        test(f44.areEqualCells(f45.getQueue().dequeue(), f44.getCellCopyByGen(1, 1, 0)));
        test(f44.areEqualCells(f47.getQueue().dequeue(), f44.getCellCopyByGen(1, 1, 0)));
        test(f44.areEqualCells(f48.getQueue().dequeue(), f44.getCellCopyByGen(1, 1, 0)));

        /* recursiveAddNeighbors() test */

        System.out.println("[OK]");
    } 

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

}
