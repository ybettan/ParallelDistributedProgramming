package ex1;

import java.util.ArrayList;




class SyncedQueue {
    
    // head --> first index
    // tail --> last index
    private ArrayList<Cell> queue;

    synchronized Cell dequeue() {
       while (queue.isEmpty()) {
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

    synchronized void enqueue(Cell c) {
        queue.add(c);
        notify();
    }

    private boolean isEmpty() {
        return queue.size() == 0;
    }

}
