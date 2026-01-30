import java.io.ByteArrayInputStream;

public class ByteArrayInputStreamSyncVerify {
    public static void main(String[] args) {
        System.out.println("Verifying ByteArrayInputStream synchronization...");

        // Test: Verify that individual method calls are synchronized
        byte[] data = new byte[1000];
        for (int i = 0; i < data.length; i++) {
            data[i] = (byte)(i % 128);  // Fill with some data
        }

        final ByteArrayInputStream stream = new ByteArrayInputStream(data);
        
        // Track the sequence of reads to verify consistency
        final int[] readSequence = new int[data.length];
        final int[] readIndex = {0};
        final Object sequenceLock = new Object();
        
        Thread[] threads = new Thread[5];
        
        for (int t = 0; t < 5; t++) {
            final int threadId = t;
            threads[t] = new Thread(() -> {
                int localCount = 0;
                int value;
                while ((value = stream.read()) != -1 && localCount < 200) { // Limit per thread
                    synchronized(sequenceLock) {
                        if (readIndex[0] < readSequence.length) {
                            readSequence[readIndex[0]] = value;
                            readIndex[0]++;
                        }
                    }
                    localCount++;
                    
                    // Small delay to increase chance of race conditions if unsynchronized
                    try {
                        Thread.sleep(0, 100); // Very small delay
                    } catch (InterruptedException e) {
                        break;
                    }
                }
                System.out.println("Thread-" + threadId + " finished, read " + localCount + " bytes");
            }, "Thread-" + t);
        }

        System.out.println("Starting threads...");
        for (Thread t : threads) {
            t.start();
        }

        try {
            for (Thread t : threads) {
                t.join();
            }
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted");
        }

        System.out.println("All threads completed.");
        System.out.println("Total bytes read: " + readIndex[0]);

        // Now let's verify that the same sequence is produced when single-threaded
        ByteArrayInputStream singleStream = new ByteArrayInputStream(data);
        int[] expectedSequence = new int[data.length];
        int expectedCount = 0;
        int value;
        while ((value = singleStream.read()) != -1 && expectedCount < data.length) {
            expectedSequence[expectedCount] = value;
            expectedCount++;
        }

        System.out.println("Single-threaded read count: " + expectedCount);

        // Check if all values were read (may be different order, but same values)
        boolean allValuesPresent = true;
        int[] valueCounts = new int[256]; // For byte values -128 to 127 mapped to 0-255
        
        // Count values from multithreaded read
        for (int i = 0; i < readIndex[0]; i++) {
            int val = readSequence[i] & 0xFF; // Convert to unsigned
            valueCounts[val]++;
        }
        
        // Subtract values from single-threaded read
        for (int i = 0; i < expectedCount; i++) {
            int val = expectedSequence[i] & 0xFF; // Convert to unsigned
            valueCounts[val]--;
        }
        
        // Check if all counts are zero
        for (int count : valueCounts) {
            if (count != 0) {
                allValuesPresent = false;
                break;
            }
        }

        System.out.println("All values present in correct amounts: " + allValuesPresent);
        
        // Also test available() method synchronization
        ByteArrayInputStream availableTestStream = new ByteArrayInputStream(data);
        System.out.println("Initial available: " + availableTestStream.available());
        
        // Test if available() method is properly synchronized
        Thread availableThread1 = new Thread(() -> {
            for (int i = 0; i < 100; i++) {
                int avail = availableTestStream.available();
                if (avail > 0) {
                    int val = availableTestStream.read();
                    if (val != -1) {
                        System.out.print(""); // Just consume
                    }
                }
            }
            System.out.println("AvailableThread1 finished");
        });

        Thread availableThread2 = new Thread(() -> {
            for (int i = 0; i < 100; i++) {
                int avail = availableTestStream.available();
                if (avail > 0) {
                    int val = availableTestStream.read();
                    if (val != -1) {
                        System.out.print(""); // Just consume
                    }
                }
            }
            System.out.println("AvailableThread2 finished");
        });

        availableThread1.start();
        availableThread2.start();
        
        try {
            availableThread1.join();
            availableThread2.join();
        } catch (InterruptedException e) {
            System.out.println("Available test interrupted");
        }
        
        System.out.println("Available method synchronization test completed.");
        System.out.println("Final available: " + availableTestStream.available());

        System.out.println("ByteArrayInputStream synchronization verification completed.");
    }
}