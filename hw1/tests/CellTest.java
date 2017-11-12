package tests;


class Cell {

    boolean isAlive;
    private int gen;
    private int checkedNeighbors;
    private int aliveNeighbors;
    // not all cells have 8 neighbors
    private int maxNeighbors;        
    // a copy of the old version for reading 
    private Cell oldVersion;

    Cell(boolean isAlive, int maxNeighbors) {
        this.isAlive = isAlive;
        gen = 0;
        checkedNeighbors = 0;
        aliveNeighbors = 0;
        this.maxNeighbors = maxNeighbors;
        // oldVersion is initiallize no null by default
    }

    // given a neighbor cell we can add it to the calculation for our cell
    void addNeighbor(Cell c) {

        //FIXME: for debugging - remove when done
        if (gen != c.gen)
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());

        checkedNeighbors++;
        if (c.isAlive) {
            aliveNeighbors++;
        }

        // if we handlend all neighbors we can update the cell to next gen
        if (checkedNeighbors == maxNeighbors) {
            moveToNextGen();
        }
    }
    
    private void moveToNextGen() {

        // FIXME: just for debugging - remove when finish
        if (aliveNeighbors > checkedNeighbors) 
            System.err.println("asserer faild at line " + 
                    new Throwable().getStackTrace()[0].getLineNumber());

        // update oldVersion to current version
        // NOTE: we update only relevant data for reading
        if (oldVersion == null) {
            // oldVersion not yet allocated
            oldVersion = new Cell(isAlive, maxNeighbors);
            // oldVersion.oldVersion recieve null by default so no loop
        }
        oldVersion.gen = gen;

        // update current version
        gen++;
        if (aliveNeighbors == 3 || (aliveNeighbors == 2 && isAlive) ) 
            isAlive = true;
        else 
            isAlive = false;
        
        checkedNeighbors = 0;
        aliveNeighbors = 0;
    }

    //FIXME: remove all -----------------------------------------------------
    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }

    static void callMain() {

        System.out.print("CellTest()...");

       // // test that my debugging assert works
       // Cell badCell = new Cell(true, 4);
       // badCell.gen = 5;
       // Cell c0 = new Cell(false, 4);
       // c0.addNeighbor(badCell);
        
        Cell c1 = new Cell(false, 4);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 
        test(c1.maxNeighbors == 4); 
        test(c1.oldVersion == null); 

        Cell deadCell = new Cell(false, 8);
        Cell liveCell = new Cell(true, 8);

        c1.addNeighbor(liveCell);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 1); 
        test(c1.aliveNeighbors == 1); 
        test(c1.oldVersion == null); 

        c1.addNeighbor(deadCell);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 2); 
        test(c1.aliveNeighbors == 1); 
        test(c1.oldVersion == null); 

        c1.addNeighbor(liveCell);
        test(c1.gen == 0); 
        test(c1.checkedNeighbors == 3); 
        test(c1.aliveNeighbors == 2); 
        test(c1.oldVersion == null); 

        c1.addNeighbor(liveCell);
        test(c1.isAlive == true); 
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 
        test(c1.oldVersion != null); 

        liveCell.gen++;
        deadCell.gen++;

        c1.addNeighbor(liveCell);
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 1); 
        test(c1.aliveNeighbors == 1); 
        test(c1.oldVersion != null); 

        c1.addNeighbor(liveCell);
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 2); 
        test(c1.aliveNeighbors == 2); 
        test(c1.oldVersion != null); 

        c1.addNeighbor(deadCell);
        test(c1.gen == 1); 
        test(c1.checkedNeighbors == 3); 
        test(c1.aliveNeighbors == 2); 
        test(c1.oldVersion != null); 

        c1.addNeighbor(deadCell);
        test(c1.isAlive == true);
        test(c1.gen == 2); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 

        liveCell.gen++;
        deadCell.gen++;

        c1.addNeighbor(liveCell);
        c1.addNeighbor(deadCell);
        c1.addNeighbor(deadCell);
        c1.addNeighbor(deadCell);
        test(c1.isAlive == false);
        test(c1.gen == 3); 
        test(c1.checkedNeighbors == 0); 
        test(c1.aliveNeighbors == 0); 

        System.out.println("[OK]");
    }
    //FIXME: ----------------------------------------------------------------
}








