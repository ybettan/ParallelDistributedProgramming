package tests;

import ex1.SyncedQueue;
import ex1.Cell;

public class SyncedQueueTest {
    
   public static void main(String args[]) {

       System.out.print("SyncedQueueTest...");

       // syngle reader single writer
       ReadObj r = new ReadObj();
       WriteObj w = new WriteObj(r.q);

       Thread readT = new Thread(r);
       Thread writeT = new Thread(w);

       readT.start();
       writeT.start();

       try {
           writeT.join();
           readT.join();
       } catch (InterruptedException e) {
           System.err.println("join ERROR, not supposed to happen");
           System.exit(1);
       }

       // signle reader multiple writers
       ReadObj2 r2 = new ReadObj2(); 
       WriteObj2 w21 = new WriteObj2(r2.q); 
       WriteObj2 w22 = new WriteObj2(r2.q); 
       WriteObj2 w23 = new WriteObj2(r2.q); 
       WriteObj2 w24 = new WriteObj2(r2.q); 
       WriteObj2 w25 = new WriteObj2(r2.q); 
       WriteObj2 w26 = new WriteObj2(r2.q); 
       WriteObj2 w27 = new WriteObj2(r2.q); 
       WriteObj2 w28 = new WriteObj2(r2.q); 

       Thread read2T = new Thread(r2);
       Thread write21T = new Thread(w21);
       Thread write22T = new Thread(w22);
       Thread write23T = new Thread(w23);
       Thread write24T = new Thread(w24);
       Thread write25T = new Thread(w25);
       Thread write26T = new Thread(w26);
       Thread write27T = new Thread(w27);
       Thread write28T = new Thread(w28);

       write21T.start();
       write22T.start();
       write23T.start();
       write24T.start();
       write25T.start();
       write26T.start();
       write27T.start();
       write28T.start();
       read2T.start();

       try {
           write21T.join();
           write22T.join();
           write23T.join();
           write24T.join();
           write25T.join();
           write26T.join();
           write27T.join();
           write28T.join();
           read2T.join();
       } catch (InterruptedException e) {
           System.err.println("join ERROR, not supposed to happen");
           System.exit(1);
       }

       System.out.println("[OK]");
   } 
}



class ReadObj implements Runnable {

    SyncedQueue q;

    ReadObj() {
        q = new SyncedQueue();
    }

    @Override public void run() {
        
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == true);    

        test(q.dequeue().isAlive == false);    
        test(q.dequeue().isAlive == false);    
        test(q.dequeue().isAlive == false);    
        test(q.dequeue().isAlive == false);    
        
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == false);    
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == false);    
        
        test(q.dequeue().isAlive == false);    
        test(q.dequeue().isAlive == false);    
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == true);    

        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == true);    
        test(q.dequeue().isAlive == false);    
    }

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }
}


class WriteObj implements Runnable {

    SyncedQueue q;

    WriteObj(SyncedQueue queue) {
        this.q = queue;
    }

    @Override public void run() {
        
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));

        q.enqueue(new Cell(false, 8));
        q.enqueue(new Cell(false, 8));
        q.enqueue(new Cell(false, 8));
        q.enqueue(new Cell(false, 8));

        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(false, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(false, 8));

        q.enqueue(new Cell(false, 8));
        q.enqueue(new Cell(false, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));

        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(false, 8));
    }
}




class ReadObj2 implements Runnable {

    SyncedQueue q;

    ReadObj2() {
        q = new SyncedQueue();
    }

    @Override public void run() {
        
        Cell last = null;
        for (int i = 0 ; i < 80 ; i++) {
            last = q.dequeue();
        }
        test(last != null);
        test(last.isAlive == false);
    }

    private static void test(boolean statment) {
        if (statment == false) 
           System.err.println("ERROR");
    }
}


class WriteObj2 implements Runnable {

    SyncedQueue q;

    WriteObj2(SyncedQueue queue) {
        this.q = queue;
    }

    @Override public void run() {
        
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(true, 8));
        q.enqueue(new Cell(false, 8));
    }
}






