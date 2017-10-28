package ex1;

public class ParallelGameOfLife implements GameOfLife {

	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit, int generations) {

        boolean[][][] results = new boolean[2][][];	

        GetGen prevGen = new GetGen(initialField, hSplit, vSplit, generations-1);
        GetGen lastGen = new GetGen(initialField, hSplit, vSplit, generations);
        
        new t0 = Thread(prevGen);
        new t1 = Thread(lastGen);
        
        t0.start();
        t1.start();

        t0.join();
        t1.join();

        results[0] = prevGen.getValue();
        results[1] = lastGen.getValue();

        
        // FIXME: maybe is it better this way for cache HR longer but better cache use
        // GetGen prevGen = new GetGen(initialField, hSplit, vSplit, generations-1);
        // prevGen.start();
        // results[0] = prevGen.getValue();
        // GetGen lastGen = new GetGen(results[0], hSplit, vSplit, 1);
        // vGen.start();
        // results[1] = lastGen.getValue();
        
    
        return results;
	}

}



class GetGen implements Runnable {
    
    boolean readField[][];
    boolean writeField[][];
    int hSplit;
    int vSplit;
    int generations;

    public GetGen(boolean[][] initalField, int hSplit, int vSplit, int generations) {
        
        this.hSplit = hSplit;
        this.vSplit = vSplit;
        this.generations = generations;

        // initial readField with initialField
		readField = new boolean[initalField.length][];
		for (int i=0 ; i<initalField.length ; i++){
			readField[i] = new boolean[initalField[0].length];
			for (int k=0 ; k<readField[i].length ; k++){
				readField[i][k]=initalField[i][k];
			}
		}

        // allocate writeField
		writeField = new boolean[initalField.length][];
		for (int i=0 ; i<initalField.length ; i++){
			readField[i] = new boolean[initalField[0].length];
		}

    }

    public boolean[][] getValue() {
        return writeField;
    }

    public void run() {
        // TODO: implement in multi-thread by matrix areas
    }
}


// FIXME: do we need this to let each thread work also in multi-thread?
//class Job implements Runnable {
//
//    public Job(boolean[][] initalField) {
//
//    }
//
//    public void run() {
//        // TODO: implement the logic of each thread
//    }
//}
