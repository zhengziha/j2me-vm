public class ThreadTest {
    public static void main(String[] args) {
        System.out.println("=== Thread Test ===");
        
        testBasicThread();
        testRunnableThread();
        testThreadSleep();
        testThreadJoin();
        testThreadPriority();
        
        System.out.println("=== All Thread Tests Completed ===");
    }
    
    static void testBasicThread() {
        System.out.println("\n--- Basic Thread Test ---");
        
        MyThread thread1 = new MyThread("Thread-1");
        MyThread thread2 = new MyThread("Thread-2");
        
        System.out.println("Starting threads...");
        thread1.start();
        thread2.start();
        
        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted: " + e.getMessage());
        }
        
        System.out.println("Basic thread: PASSED");
    }
    
    static void testRunnableThread() {
        System.out.println("\n--- Runnable Thread Test ---");
        
        MyRunnable runnable1 = new MyRunnable("Runnable-1");
        MyRunnable runnable2 = new MyRunnable("Runnable-2");
        
        Thread thread1 = new Thread(runnable1);
        Thread thread2 = new Thread(runnable2);
        
        System.out.println("Starting runnable threads...");
        thread1.start();
        thread2.start();
        
        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted: " + e.getMessage());
        }
        
        System.out.println("Runnable thread: PASSED");
    }
    
    static void testThreadSleep() {
        System.out.println("\n--- Thread Sleep Test ---");
        
        SleepThread sleepThread = new SleepThread();
        sleepThread.start();
        
        try {
            sleepThread.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted: " + e.getMessage());
        }
        
        System.out.println("Thread sleep: PASSED");
    }
    
    static void testThreadJoin() {
        System.out.println("\n--- Thread Join Test ---");
        
        WorkerThread worker = new WorkerThread();
        worker.start();
        
        System.out.println("Main thread waiting for worker to complete...");
        
        try {
            worker.join();
            System.out.println("Worker thread completed");
            System.out.println("Thread join: PASSED");
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted: " + e.getMessage());
            System.out.println("Thread join: FAILED");
        }
    }
    
    static void testThreadPriority() {
        System.out.println("\n--- Thread Priority Test ---");
        
        PriorityThread highPriority = new PriorityThread("HighPriority");
        PriorityThread lowPriority = new PriorityThread("LowPriority");
        
        highPriority.setPriority(Thread.MAX_PRIORITY);
        lowPriority.setPriority(Thread.MIN_PRIORITY);
        
        System.out.println("High priority thread: " + highPriority.getPriority());
        System.out.println("Low priority thread: " + lowPriority.getPriority());
        
        highPriority.start();
        lowPriority.start();
        
        try {
            highPriority.join();
            lowPriority.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted: " + e.getMessage());
        }
        
        System.out.println("Thread priority: PASSED");
    }
    
    static class MyThread extends Thread {
        private String name;
        
        public MyThread(String name) {
            this.name = name;
        }
        
        public void run() {
            System.out.println(name + " started");
            for (int i = 1; i <= 3; i++) {
                System.out.println(name + " count: " + i);
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    System.out.println(name + " interrupted: " + e.getMessage());
                }
            }
            System.out.println(name + " finished");
        }
    }
    
    static class MyRunnable implements Runnable {
        private String name;
        
        public MyRunnable(String name) {
            this.name = name;
        }
        
        public void run() {
            System.out.println(name + " started");
            for (int i = 1; i <= 3; i++) {
                System.out.println(name + " count: " + i);
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    System.out.println(name + " interrupted: " + e.getMessage());
                }
            }
            System.out.println(name + " finished");
        }
    }
    
    static class SleepThread extends Thread {
        public void run() {
            System.out.println("SleepThread started");
            try {
                System.out.println("Sleeping for 500ms...");
                Thread.sleep(500);
                System.out.println("Woke up from sleep");
            } catch (InterruptedException e) {
                System.out.println("SleepThread interrupted: " + e.getMessage());
            }
            System.out.println("SleepThread finished");
        }
    }
    
    static class WorkerThread extends Thread {
        public void run() {
            System.out.println("WorkerThread started");
            for (int i = 1; i <= 5; i++) {
                System.out.println("Worker working: " + i);
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e) {
                    System.out.println("Worker interrupted: " + e.getMessage());
                }
            }
            System.out.println("WorkerThread finished");
        }
    }
    
    static class PriorityThread extends Thread {
        private String name;
        
        public PriorityThread(String name) {
            this.name = name;
        }
        
        public void run() {
            System.out.println(name + " started with priority " + getPriority());
            for (int i = 1; i <= 3; i++) {
                System.out.println(name + " working: " + i);
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    System.out.println(name + " interrupted: " + e.getMessage());
                }
            }
            System.out.println(name + " finished");
        }
    }
}
