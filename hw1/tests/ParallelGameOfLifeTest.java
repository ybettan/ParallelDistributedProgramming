package tests;

import ex1.Field;
import ex1.Neighbors;
import ex1.ParallelGameOfLife;

class field_stages {
    boolean[][][] all_dead = {
            {
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false}
            }, {
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false}
            }, {
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false}
            }
    };
    boolean[][][] across = {
            {
                    {true, false, false, false},
                    {false, true, false, false},
                    {false, false, true, false},
                    {false, false, false, true}
            }, {
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false}
            }, {
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false},
                    {false, false, false, false}
            }
    };
    boolean[][][] stuckInTheMiddle = {
            {
                    {false, false, false, false},
                    {false, true, true, false},
                    {false, true, true, false},
                    {false, false, false, false}
            }, {
                    {false, false, false, false},
                    {false, true, true, false},
                    {false, true, true, false},
                    {false, false, false, false}
            }, {
                    {false, false, false, false},
                    {false, true, true, false},
                    {false, true, true, false},
                    {false, false, false, false}
        }
    };
    boolean[][][] row1 = {
        {
                    {false, false, false, false }
        }, {
                    {false, false, false, false }
        }, {
                    {false, false, false, false }
        }
    };
    boolean[][][] row2 = {
            {
                    {true, true, true, true }
            }, {
                    {false, false, false, false }
            }, {
                    {false, false, false, false }
            }
    };
    boolean[][][] column1 = {
            {
                    {false},
                    {false},
                    {false},
                    {false}
            }, {
                    {false},
                    {false},
                    {false},
                    {false}
            }, {
                    {false},
                    {false},
                    {false},
                    {false}
            }
    };
    boolean[][][] column2 = {
            {
                    {true},
                    {true},
                    {true},
                    {true}
            }, {
                    {false},
                    {false},
                    {false},
                    {false}
            }, {
                    {false},
                    {false},
                    {false},
                    {false}
            }
    };
    field_stages() { }
}
public class ParallelGameOfLifeTest {

    ParallelGameOfLifeTest(){}

    public static boolean test(int vSplit, int hSplit, boolean[][][] fieldGenerations) {

        ParallelGameOfLife pGOF = new ParallelGameOfLife();
        int maxGeneration = 2;

        // set the parameters.
        pGOF.sethSplit(hSplit);
        pGOF.setvSplit(vSplit);

        // test the allocation of the result.
        boolean[][][] res = pGOF.allocateGofResult(fieldGenerations[0]);

        assert (res.length == 2);
        assert (res[0].length == fieldGenerations[0].length);
        assert (res[0][0].length == fieldGenerations[0][0].length);

        // test the creation of fields.
        pGOF.allocateAndCreateFields(fieldGenerations[0], res, maxGeneration);
        pGOF.applyNeighbors();
        Field[][] f = pGOF.getThreadLocations();
        assert (f != null);
        assert (f.length == vSplit);
        assert (f[0].length == hSplit);
        Field ful = f[0][0];
        Neighbors nul = ful.getNeighbors();

        assert (nul.getUp() == null);
        assert (nul.getLeft() == null);
        assert (nul.getUpLeft() == null);

        if ((f.length > 2 )  && (f[0].length > 2)) {
            Neighbors n = f[1][1].getNeighbors();
            assert (n.getUp() == f[0][1]);
            assert (n.getUpLeft() == f[0][0]);
            assert (n.getUpRight() == f[0][2]);
            assert (n.getLeft() == f[1][0]);
            assert (n.getRight() == f[1][2]);
            assert (n.getDownLeft() == f[2][0]);
            assert (n.getDown() == f[2][1]);
            assert (n.getDownRight() == f[2][2]);
        }

        /* pGOF.startGOF();
        for (int i = 0; i < fieldGenerations[0].length; i++){
            for (int j = 0; j < fieldGenerations[0][0].length; j++){
                assert (res[0][i][j] == fieldGenerations[maxGeneration-1][i][j]);
                assert (res[1][i][j] == fieldGenerations[maxGeneration][i][j]);
            }

        }
        */
        return true;
    }
    public static void main(String args[]) {

        System.out.print("ParallelGameOfLifeTest...");
        field_stages fs = new field_stages();
        ParallelGameOfLifeTest.test(4,4, fs.all_dead );

    }
}
