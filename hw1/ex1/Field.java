package ex1;

import ex1.SyncedQueue;


public class Field implements Runnable {

    private boolean[][] field;
    private SyncedQueue queue;
    // Field is the owner of thins array
    private Field[] neighbors; 
    
    // FIXME: make sure that ParallelGameOfLife don't keep a copy of:
    //        * neighbors
    //        * initalField 
    public Field(boolean[][] initalField, Field[] neighbors) {
        this.field = initalField;
        queue = new SyncedQueue();
        this.neighbors = neighbors;
    }

    @Override public void run() {

    } 
}

