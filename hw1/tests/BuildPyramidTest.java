package tests;

import ex1.Field;
import ex1.ParallelGameOfLife;
import ex1.Cell3D;

public class BuildPyramidTest {
    // each row is 24 long
    static boolean[][][] row =
            {
                    {
                            {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
                    },
                    {
                            {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
                    },
                    {
                            {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
                    }
            };
    // each column is 24 long
    static boolean[][][] column =
            {
                    // generation  :
                    {
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false}
                    },
                    // generation  :
                    {
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false}
                    },
                    // generation  :
                    {
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false},
                            {false}
                    }
            };
    // dead pyramid 12 x 12.
    static boolean[][][] deadPyramid =
            {
                    // generation 0:
                    {
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false}
                    },
                    // generation 1:
                    {
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false}
                    },
                    // generation 3:
                    {
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false},
                            {false, false, false, false, false, false, false, false, false, false, false, false}
                    }
            };
    /*
    test the dimensions of the pyramid.
    each field is 4 x 4, which means that after building a pyramid the generations
    should be according.
     */
    private static void testDimensions(){
        System.out.println("testDimensions...");
        // check that it builds a pyramid that leans out.
        ParallelGameOfLife pgof = new ParallelGameOfLife();
        pgof.setvSplit(3);
        pgof.sethSplit(3);
        boolean[][][] res = pgof.allocateGofResult(deadPyramid[0]);
        pgof.allocateAndCreateFields(deadPyramid[0],res,3);
        pgof.applyNeighbors();
        Field[][] ff = pgof.getThreadLocations();
        for (int i = 0; i < ff.length; i++)
            for (int j = 0; j < ff[0].length; j++)
                ff[i][j].buildPyramid(0,4,0,4,0);


        System.out.println("\tUp Left Field...");
        Field f = ff[0][0];
        Cell3D c3d = f.field[0][0];
        assert (c3d.getGenOfCurrent() == 3);
        c3d = f.field[0][1];
        assert (c3d.getGenOfCurrent() == 2);
        c3d = f.field[1][1];
        assert (c3d.getGenOfCurrent() == 2);
        c3d = f.field[1][0];
        assert (c3d.getGenOfCurrent() == 2);
        c3d = f.field[0][2];
        assert (c3d.getGenOfCurrent() == 1);
        c3d = f.field[2][0];
        assert (c3d.getGenOfCurrent() == 1);
        c3d = f.field[1][2];
        assert (c3d.getGenOfCurrent() == 1);
        c3d = f.field[2][2];
        assert (c3d.getGenOfCurrent() == 1);
        c3d = f.field[2][1];
        assert (c3d.getGenOfCurrent() == 1);
        c3d = f.field[0][3];
        assert (c3d.getGenOfCurrent() == 0);
        c3d = f.field[1][3];
        assert (c3d.getGenOfCurrent() == 0);
        c3d = f.field[2][3];
        assert (c3d.getGenOfCurrent() == 0);
        c3d = f.field[3][3];
        assert (c3d.getGenOfCurrent() == 0);
        c3d = f.field[3][2];
        assert (c3d.getGenOfCurrent() == 0);
        c3d = f.field[3][1];
        assert (c3d.getGenOfCurrent() == 0);
        c3d = f.field[3][0];
        assert (c3d.getGenOfCurrent() == 0);

        System.out.println("\tUp Field...");
        f = ff[0][1];

        for (int i =0; i < 4; i++){
            c3d = f.field[3][i];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[i][0];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[i][3];
            assert (c3d.getGenOfCurrent() == 0);
        }
        for (int i =0; i < 3; i++){
            c3d = f.field[i][1];
            assert (c3d.getGenOfCurrent() == 1);
            c3d = f.field[i][2];
            assert (c3d.getGenOfCurrent() == 0);
        }
        System.out.println("\tLeft Field...");
        f = ff[1][0];
        for (int i =0; i < 4; i++){
            c3d = f.field[0][i];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[i][3];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[3][i];
            assert (c3d.getGenOfCurrent() == 0);
        }
        for (int i =0; i < 3; i++){
            c3d = f.field[1][i];
            assert (c3d.getGenOfCurrent() == 1);
            c3d = f.field[2][i];
            assert (c3d.getGenOfCurrent() == 0);
        }
        System.out.println("\tRight Field...");
        f = ff[1][2];
        for (int i =0; i < 4; i++){
            c3d = f.field[0][i];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[i][0];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[3][i];
            assert (c3d.getGenOfCurrent() == 0);
        }
        for (int i =1; i < 4; i++){
            c3d = f.field[1][i];
            assert (c3d.getGenOfCurrent() == 1);
            c3d = f.field[2][i];
            assert (c3d.getGenOfCurrent() == 0);
        }
        System.out.println("\tDown Field...");
        f = ff[2][1];
        for (int i =0; i < 4; i++){
            c3d = f.field[0][i];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[i][0];
            assert (c3d.getGenOfCurrent() == 0);
            c3d = f.field[3][i];
            assert (c3d.getGenOfCurrent() == 0);
        }
        for (int i =0; i < 3; i++){
            c3d = f.field[i][1];
            assert (c3d.getGenOfCurrent() == 1);
            c3d = f.field[i][2];
            assert (c3d.getGenOfCurrent() == 0);
        }

        System.out.println("\tUp Right Field...");
        f = ff[0][2];
        for (int i = 0; i < 4; i++){
            c3d = f.field[3-i][i];
            assert(c3d.getGenOfCurrent() == i);
        }

        System.out.println("\tDown Left Field...");
        f = ff[2][0];
        for (int i = 0; i < 4; i++){
            c3d = f.field[3-i][i];
            assert(c3d.getGenOfCurrent() == 3-i);
        }
        System.out.println("\tDown Right Field...");
        f = ff[2][2];
        for (int i = 0; i < 4; i++){
            c3d = f.field[i][i];
            assert(c3d.getGenOfCurrent() == i);
        }
        System.out.println("\tCenter Field...");
        for (int i =0; i < 4; i++)
            for (int j = 0; j < 4; j++){
                c3d = f.field[i][j];
                if (i==0 || i==3 || j==0 || j==3) {
                    assert (c3d.getGenOfCurrent() == 0);
                } else {
                    assert (c3d.getGenOfCurrent() == 1);
                }
            }
        System.out.println("~~~[OK}~~~");
    }


    static void testMaxGen() {
        System.out.println("testMaxGen...");
        // check that it builds a pyramid that leans out.
        ParallelGameOfLife pgof = new ParallelGameOfLife();
        pgof.setvSplit(1);
        pgof.sethSplit(1);
        boolean[][][] res = pgof.allocateGofResult(deadPyramid[0]);
        pgof.allocateAndCreateFields(deadPyramid[0],res,20);
        pgof.applyNeighbors();
        Field[][] ff = pgof.getThreadLocations();
        for (int i = 0; i < ff.length; i++)
            for (int j = 0; j < ff[0].length; j++)
                ff[i][j].buildPyramid(0,4,0,4,0);
        Field f = ff[0][0];
        Cell3D c3d;
        for (int i =0; i < f.field.length; i++)
            for (int j = 0; j < f.field[0].length; j++){
                c3d = f.field[i][j];
                assert (c3d.getGenOfCurrent() == 20);
            }
        System.out.println("~~~[OK}~~~");
    }

    static void ColumnTest() {
        System.out.println("ColumnTest...");
        // check that it builds a pyramid that leans out.
        ParallelGameOfLife pgof = new ParallelGameOfLife();
        pgof.setvSplit(3);
        pgof.sethSplit(3);
        boolean[][][] res = pgof.allocateGofResult(column[0]);
        pgof.allocateAndCreateFields(deadPyramid[0],res,20);
        pgof.applyNeighbors();
        Field[][] ff = pgof.getThreadLocations();
        for (int i = 0; i < ff.length; i++)
            for (int j = 0; j < ff[0].length; j++)
                ff[i][j].buildPyramid(0,4,0,4,0);
        System.out.println("\tTest Up...");
        Field f = ff[0][0];
        Cell3D c3d;
        for (int i=0; i < f.field.length; i++){
            c3d = f.field[i][0];
            assert (c3d.getGenOfCurrent() == 7-i);
        }
        System.out.println("\tTest Middle...");
        f = ff[1][0];
        for (int i=0; i < f.field.length; i++){
            c3d = f.field[i][0];
            if (i < 4)
                assert (c3d.getGenOfCurrent() == i);
            else
                assert (c3d.getGenOfCurrent() == -i % 7);
        }


        System.out.println("\tTest Down...");
        f = ff[2][0];
        for (int i=0; i < f.field.length; i++){
            c3d = f.field[i][0];
            assert (c3d.getGenOfCurrent() == i);
        }

        System.out.println("~~~[OK}~~~");
    }
    private static void RowTest() {
        System.out.println("RowTest...");
        // check that it builds a pyramid that leans out.
        ParallelGameOfLife pgof = new ParallelGameOfLife();
        pgof.setvSplit(3);
        pgof.sethSplit(3);
        boolean[][][] res = pgof.allocateGofResult(row[0]);
        pgof.allocateAndCreateFields(deadPyramid[0],res,20);
        pgof.applyNeighbors();
        Field[][] ff = pgof.getThreadLocations();
        for (int i = 0; i < ff.length; i++)
            for (int j = 0; j < ff[0].length; j++)
                ff[i][j].buildPyramid(0,4,0,4,0);
        System.out.println("\tTest Left...");
        Field f = ff[0][0];
        Cell3D c3d;
        for (int i=0; i < f.field.length; i++){
            c3d = f.field[0][i];
            assert (c3d.getGenOfCurrent() == 7-i);
        }

        System.out.println("\tTest Middle...");
        f = ff[0][1];
        for (int i=0; i < f.field.length; i++){
            c3d = f.field[0][i];
            if (i < 4)
                assert (c3d.getGenOfCurrent() == i);
            else
                assert (c3d.getGenOfCurrent() == -i % 7);
        }

        System.out.println("\tTest Right...");
        f = ff[0][2];
        for (int i=0; i < f.field.length; i++){
            c3d = f.field[0][i];
            assert (c3d.getGenOfCurrent() == i);
        }
        System.out.println("~~~[OK}~~~");
    }
    public static void main(String args[]){
        BuildPyramidTest.testDimensions();
        BuildPyramidTest.testMaxGen();
        BuildPyramidTest.ColumnTest();
        BuildPyramidTest.RowTest();

    }
}
