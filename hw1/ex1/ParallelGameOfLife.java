package ex1;

import java.util.ArrayList;




public class ParallelGameOfLife implements GameOfLife {

	public boolean[][][] invoke(boolean[][] initalField, int hSplit, int vSplit,
			int generations) {

	    boolean[][][] x=new boolean[2][][];	
        ArrayList<ArrayList<ArrayList<Boolean>>> field;

        // give each thread a parthial-copy of the initalField
        field = divideField(initalField, hSplit, vSplit);

		x[0] = get_gen(initalField,hSplit,vSplit, generations-1);
		x[1] = get_gen(initalField,hSplit,vSplit, generations);
        
		return x;
	}

	private boolean[][] get_gen(boolean[][] initalField, int hSplit, int vSplit, int generations) {
	
		// init the field -> copy the field to the local array
		boolean[][] input=new boolean[initalField.length][];
		
		for (int i=0;i<initalField.length;i++){
			input[i]=new boolean[initalField[0].length];
			for (int k=0;k<input[i].length;k++){
				input[i][k]=initalField[i][k];
			}
		}

		boolean[][] result=new boolean[initalField.length][];
		
		for (int g=0;g<generations;g++){
			for (int i=0;i<initalField.length;i++){
				if (result[i]==null){
					// using first time -> copy the array
					result[i]=new boolean[initalField[i].length];
				}
				
				for (int j=0;j<initalField[i].length;j++){
					int numNeighbors=numNeighbors(i,j,input);
					result[i][j]=false;
					if (numNeighbors==3 || (input[i][j]&& numNeighbors==2)){
						result[i][j]=true;
					}
				}
			}
			boolean[][] tmp;
			tmp=input;
			input=result;
			result=tmp;
		}
		return input;
	}
	
	private int numNeighbors(int x,int y, boolean[][] field ){
		int counter=(field[x][y]?-1:0);
		for (int i=x-1; i<=x+1;i++ ){
			if (i<0||i>=field.length){ continue ; }
			for (int j=y-1; j<=y+1;j++){
				if (j<0||j>=field[0].length){ continue ; }
				counter+=(field[i][j]?1:0);
			}
		}
		return counter;
	}

    // FIXME: update to 2D array if autorized
    // this function create an ArrayList for all threads
    // each object in the ArrayList is a 2D ArrayList (a part of the field)
    // we cannot use an array of 2D ArraysLists cause arrays don't hold
    //                                                       generic types
    private ArrayList<ArrayList<ArrayList<Boolean>>> 
    divideField(boolean[][] initalField, int hSplit, int vSplit) {
        
        int numOfThread = hSplit * vSplit;

        // create the ArrayList
        ArrayList<ArrayList<ArrayList<Boolean>>> fields = new ArrayList<>();

        // for each thread copy its relevant part of initalField
        for (int i = 0 ; i < numOfThread ; i++) 
            fields.add(copyFromOrigin(initalField, i, hSplit, vSplit));
        
        return fields;
    }

    // FIXME: update to 2D array if autorized
    private ArrayList<ArrayList<Boolean>> 
    copyFromOrigin(boolean[][] initalField, int threadIndex, int hSplit, 
            int vSplit) {

        int hSplitSize = initalField[0].length / hSplit; 
        int vSplitSize = initalField.length / vSplit;
        
        int row = (threadIndex / hSplit ) * vSplitSize; 
        int col = (threadIndex % hSplit) * hSplitSize; 

        // if the field didn't split equally so the remider is for the last one
        int rowReminder = initalField.length % vSplit;
        if (rowReminder != 0 && row / vSplitSize == vSplit-1) {
            vSplitSize += rowReminder;
        }

        // if the field didn't split equally so the remider is for the last one
        int colReminder = initalField[0].length % hSplit;
        if (colReminder != 0 && threadIndex % hSplit == hSplit-1) {
            hSplitSize += colReminder;
        }

        ArrayList<ArrayList<Boolean>> field = new ArrayList<>();
        for (int i = 0 ; i < vSplitSize ; i++) {
            field.add(new ArrayList<Boolean>());
            for (int j = 0 ; j < hSplitSize ; j++) {
                field.get(i).add(initalField[row + i][col + j]);
            }
        }
        return field;
    }
}


























