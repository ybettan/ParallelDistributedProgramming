package ex1;

import java.util.ArrayList;

/*
 SyncedQueue is used to create a consumer prodocer algorithem between the threads of game of life.
 each Cell that is ready for his neighbors, when the neighbors belong to different thread, are passed by this Queue.
 */
public class SyncedQueue {
    // head --> first index
    // tail --> last index
    private ArrayList<Cell> queue;

    /*
    C'tor.
     */
    public SyncedQueue() {
        queue = new ArrayList<Cell>();
    }

    /*
    Dequeue - if the Queue is empty the thread will wait on it untill another thread will push a Cell and wake him up.
    Consumer.
     */
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

    /*
    Enqueue add a cell to the queueu. if the thread is waiting for data the pusher will wake him up.
    Producer.
     */
    public synchronized void enqueue(Cell c) {
        queue.add(c);
        notify();
    }
}
