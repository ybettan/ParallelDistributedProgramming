package ex1;

import java.util.ArrayList;




public class SyncedQueue {
    
    // head --> first index
    // tail --> last index
    private ArrayList<Cell> queue;

    public SyncedQueue() {
        queue = new ArrayList<Cell>();
    }

    public synchronized Cell dequeue() {
       while (queue.size() == 0) {
           try {
               wait();            
           } catch (InterruptedException e) {
               System.err.println("wait ERROR, not supposed to hapen");
               System.exit(1);
           }
       }
       Cell head = queue.get(0);
       queue.remove(0);
       return head;
    }

    public synchronized void enqueue(Cell c) {
        queue.add(c);
        notify();
    }
}
