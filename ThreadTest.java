public class ThreadTest implements Runnable {
    public static void main(String[] args) {
        System.out.println("Main start");
        
        // Case 1: Runnable (using self)
        System.out.println("Test Runnable:");
        new Thread(new ThreadTest()).start();
        
        System.out.println("Main end");
    }
    
    public void run() {
        System.out.println("Runnable running");
    }
}
