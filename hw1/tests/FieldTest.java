package tests;

import ex1.Field;
import ex1.Neighbors;


public class FieldTest {

    public static void main(String args[]) {

        System.out.print("FieldTest...");

       // boolean[][] initalField = { {false, false, false, false, false, false},
       //                             {false, false, false, false, false, false},
       //                             {false, false, true, true, false, false},
       //                             {false, false, true, true, false, false},
       //                             {false, false, false, false, false, false},
       //                             {false, false, false, false, false, false} };
        boolean[][] initalField = { {false, false, true, true, false, false},
                                    {false, false, true, true, false, false},
                                    {true, true, false, false, true, true},
                                    {true, true, false, false, true, true},
                                    {false, false, true, true, false, false},
                                    {false, false, true, true, false, false} };
        
        boolean[][][] resultField = new boolean[2][][];
        resultField[0] = new boolean[6][];
        resultField[1] = new boolean[6][];
            for (int i=0 ; i<6 ; i++) {
                resultField[0][i] = new boolean[6];
                resultField[1][i] = new boolean[6];
            }

        /* check logical partition */
        Field f0 = new Field(initalField, 0, 1, 0, 1, 1, resultField);
        Field f1 = new Field(initalField, 0, 1, 2, 3, 1, resultField);
        Field f2 = new Field(initalField, 0, 1, 4, 5, 1, resultField);
        Field f3 = new Field(initalField, 2, 3, 0, 1, 1, resultField);
        Field f4 = new Field(initalField, 2, 3, 2, 3, 1, resultField);
        Field f5 = new Field(initalField, 2, 3, 4, 5, 1, resultField);
        Field f6 = new Field(initalField, 4, 5, 0, 1, 1, resultField);
        Field f7 = new Field(initalField, 4, 5, 2, 3, 1, resultField);
        Field f8 = new Field(initalField, 4, 5, 4, 5, 1, resultField);

        //System.out.println();
        //f0.printField();
        //f1.printField();
        //f2.printField();
        //f3.printField();
        //f4.printField();
        //f5.printField();
        //f6.printField();
        //f7.printField();
        //f8.printField();

        f0.setNeighbors(new Neighbors(null, null, f1, f4, f3, null, null, null));
        f1.setNeighbors(new Neighbors(null, null, f2, f5, f4, f3, f0, null));
        f2.setNeighbors(new Neighbors(null, null, null, null, f5, f4, f1, null));
        f3.setNeighbors(new Neighbors(f0, f1, f4, f7, f6, null, null, null));
        f4.setNeighbors(new Neighbors(f1, f2, f5, f8, f7, f6, f3, f0));
        f5.setNeighbors(new Neighbors(f2, null, null, null, f8, f7, f4, f1));
        f6.setNeighbors(new Neighbors(f3, f4, f7, null, null, null, null, null));
        f7.setNeighbors(new Neighbors(f4, f5, f8, null, null, null, f6, f3));
        f8.setNeighbors(new Neighbors(f5, null, null, null, null, null, f7, f4));

        /* check communication part on f0-f8 */

        /* check autonumus part on a field with granularity == 1 */
        Field f9 = new Field(initalField, 0, 5, 0, 5, 1, resultField);
        f9.setNeighbors(new Neighbors(null, null, null, null, null, null, null, null));
        f9.run();
        


        System.out.println("[OK]");
    } 

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

}
