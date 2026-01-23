package java.lang;

public class Thread implements Runnable {
    private Runnable target;
    private boolean alive = false;

    public Thread() {
    }

    public Thread(Runnable target) {
        this.target = target;
    }

    public Thread(String name) {
    }

    public Thread(Runnable target, String name) {
        this.target = target;
    }

    public void start() {
        if (alive) {
             throw new IllegalThreadStateException();
        }
        alive = true;
        start0();
    }

    private native void start0();

    public void run() {
        if (target != null) {
            target.run();
        }
    }

    public static Thread currentThread() {
        return new Thread();
    }

    public static native void sleep(long millis) throws InterruptedException;

    public static native void yield();

    public static void sleep(long millis, int nanos) throws InterruptedException {
        // Ignore nanos for now
        sleep(millis);
    }

    public boolean isAlive() {
        return alive;
    }

    public void interrupt() {
    }

    public boolean isInterrupted() {
        return false;
    }

    public static boolean interrupted() {
        return false;
    }

    public void join() throws InterruptedException {
    }

    public void join(long millis) throws InterruptedException {
    }

    public void join(long millis, int nanos) throws InterruptedException {
    }

    public void setPriority(int newPriority) {
    }

    public int getPriority() {
        return 5;
    }

    public void setName(String name) {
    }

    public String getName() {
        return "Thread-" + hashCode();
    }

    public void setDaemon(boolean on) {
    }

    public boolean isDaemon() {
        return false;
    }
}
