import java.io.ByteArrayInputStream;

/**
 * 测试ByteArrayInputStream的同步问题
 */
class SyncWorker implements Runnable {
    private ByteArrayInputStream stream;
    private String workerName;
    private volatile boolean finished = false;

    public SyncWorker(ByteArrayInputStream stream, String workerName) {
        this.stream = stream;
        this.workerName = workerName;
    }

    @Override
    public void run() {
        try {
            System.out.println(workerName + " starting...");
            int bytesRead = 0;
            int value;
            while ((value = stream.read()) != -1 && bytesRead < 20) {
                System.out.println(workerName + " read: " + value);
                bytesRead++;
                // 模拟一些处理时间
                try {
                    Thread.sleep(1);
                } catch (InterruptedException e) {
                    break;
                }
            }
            System.out.println(workerName + " finished reading " + bytesRead + " bytes");
        } catch (Exception e) {
            System.out.println(workerName + " encountered exception: " + e.getMessage());
        } finally {
            finished = true;
        }
    }

    public boolean isFinished() {
        return finished;
    }
}

public class ByteArrayInputStreamSyncTest {
    public static void main(String[] args) {
        System.out.println("Testing ByteArrayInputStream synchronization issues...");
        
        // 创建共享的数据源
        byte[] sharedData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
        ByteArrayInputStream sharedStream = new ByteArrayInputStream(sharedData);
        
        System.out.println("Shared data: ");
        for (int i = 0; i < sharedData.length; i++) {
            System.out.print(sharedData[i] + " ");
        }
        System.out.println();
        
        // 创建两个线程同时访问同一个ByteArrayInputStream
        SyncWorker worker1 = new SyncWorker(sharedStream, "Worker-1");
        SyncWorker worker2 = new SyncWorker(sharedStream, "Worker-2");
        
        Thread thread1 = new Thread(worker1);
        Thread thread2 = new Thread(worker2);
        
        System.out.println("Starting threads...");
        thread1.start();
        thread2.start();
        
        // 等待线程完成
        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted");
        }
        
        System.out.println("Both threads completed.");
        
        // 重新创建流以验证数据是否正确读取
        ByteArrayInputStream verifyStream = new ByteArrayInputStream(sharedData);
        System.out.println("Verifying stream state after concurrent access:");
        int value;
        int count = 0;
        while ((value = verifyStream.read()) != -1 && count < 10) {
            System.out.print(value + " ");
            count++;
        }
        System.out.println();
        
        System.out.println("Testing mark/reset in multithreaded context...");
        byte[] testData = {10, 20, 30, 40, 50};
        ByteArrayInputStream markTestStream = new ByteArrayInputStream(testData);
        
        // Mark position
        markTestStream.mark(0);
        System.out.println("Position after mark: " + (testData.length - markTestStream.available()));
        
        // Read some data
        int val1 = markTestStream.read();
        int val2 = markTestStream.read();
        System.out.println("Read values: " + val1 + ", " + val2);
        System.out.println("Position after reads: " + (testData.length - markTestStream.available()));
        
        // Reset to marked position
        markTestStream.reset();
        System.out.println("Position after reset: " + (testData.length - markTestStream.available()));
        
        int val3 = markTestStream.read();
        System.out.println("Value after reset and read: " + val3);
        
        System.out.println("ByteArrayInputStream sync test completed.");
    }
}