public class SyncTest {
    private static int counter = 0;
    private static final Object lock = new Object();

    public static void main(String[] args) {
        System.out.println("Testing synchronization mechanisms...");

        // Test 1: Synchronized method
        Thread thread1 = new Thread(new CounterRunnable(), "Thread-1");
        Thread thread2 = new Thread(new CounterRunnable(), "Thread-2");

        System.out.println("Starting synchronized method test...");
        thread1.start();
        thread2.start();

        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted");
        }

        System.out.println("Final counter value: " + counter + " (should be 200000 if synchronized correctly)");
        System.out.println("Expected: 200000, Actual: " + counter);

        // Reset counter for next test
        counter = 0;

        // Test 2: Synchronized block
        Thread thread3 = new Thread(new BlockCounterRunnable(), "Block-Thread-1");
        Thread thread4 = new Thread(new BlockCounterRunnable(), "Block-Thread-2");

        System.out.println("\nStarting synchronized block test...");
        thread3.start();
        thread4.start();

        try {
            thread3.join();
            thread4.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted");
        }

        System.out.println("Final counter value: " + counter + " (should be 200000 if synchronized correctly)");
        System.out.println("Expected: 200000, Actual: " + counter);

        System.out.println("Synchronization test completed.");
    }

    static class CounterRunnable implements Runnable {
        @Override
        public void run() {
            for (int i = 0; i < 100000; i++) {
                incrementCounter();
            }
            System.out.println(Thread.currentThread().getName() + " finished");
        }

        private synchronized void incrementCounter() {
            int temp = counter;
            // Simulate some processing time
            for (int j = 0; j < 10; j++) {
                // Busy work
            }
            counter = temp + 1;
        }
    }

    static class BlockCounterRunnable implements Runnable {
        @Override
        public void run() {
            for (int i = 0; i < 100000; i++) {
                synchronized (lock) {
                    int temp = counter;
                    // Simulate some processing time
                    for (int j = 0; j < 10; j++) {
                        // Busy work
                    }
                    counter = temp + 1;
                }
            }
            System.out.println(Thread.currentThread().getName() + " finished");
        }
    }
}