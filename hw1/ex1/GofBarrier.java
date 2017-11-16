package ex1;

/*
 * Gof Barrier will help to make sure that all the fields passed initial or final stages together.
 * it will not be used to sync generations, but it can be used to make sure that all the threads
 * finished copping their initial board, hence they can start the game.
 */
public class GofBarrier {
    private int currentCount;
    private int targetCount;

    GofBarrier(int targetThreadCount) {
        currentCount = 0;
        targetCount = targetThreadCount;
    }

    public synchronized int getCurrentCount() {
        return currentCount;
    }

    public synchronized int getTargetCount() {
        return targetCount;
    }

    /*
     * By resetting the barriers limit\target, all threads that are waiting on it will be awoken.
     */
    public synchronized void setTargetCount(int targetCount) {
        this.currentCount = 0;
        this.targetCount = targetCount;
        notifyAll();
    }

    public synchronized void waitForAll() throws InterruptedException {
        this.targetCount++;
        if (this.currentCount == this.targetCount) {
            this.currentCount = 0;
            notifyAll();
        } else {
            wait();
        }
    }
}
