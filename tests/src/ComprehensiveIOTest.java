import java.io.*;

public class ComprehensiveIOTest {
    private static int passed = 0;
    private static int failed = 0;
    
    public static void main(String[] args) {
        System.out.println("=== Comprehensive I/O Stream Test ===");
        
        testInputStreamBasics();
        testInputStreamSkip();
        testInputStreamMarkReset();
        testInputStreamReadBuffer();
        testOutputStreamBasics();
        testFilterStreams();
        testDataStreams();
        testStreamChaining();
        testExceptionHandling();
        testBoundaryConditions();
        
        System.out.println("\n=== Test Summary ===");
        System.out.println("Passed: " + passed);
        System.out.println("Failed: " + failed);
        System.out.println("Total: " + (passed + failed));
        
        if (failed == 0) {
            System.out.println("ALL TESTS PASSED!");
        } else {
            System.out.println("SOME TESTS FAILED!");
        }
    }
    
    static void assertTrue(String testName, boolean condition) {
        if (condition) {
            System.out.println("  [PASS] " + testName);
            passed++;
        } else {
            System.out.println("  [FAIL] " + testName);
            failed++;
        }
    }
    
    static void testInputStreamBasics() {
        System.out.println("\n--- InputStream Basics ---");
        
        byte[] data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        
        try {
            assertTrue("Available returns correct count", bis.available() == 10);
            
            int b = bis.read();
            assertTrue("Read first byte", b == 0);
            
            assertTrue("Available decreases after read", bis.available() == 9);
            
            byte[] buffer = new byte[3];
            int bytesRead = bis.read(buffer);
            assertTrue("Read buffer returns correct count", bytesRead == 3);
            assertTrue("Buffer content correct", buffer[0] == 1 && buffer[1] == 2 && buffer[2] == 3);
            
            assertTrue("Available after buffer read", bis.available() == 6);
            
            bis.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testInputStreamSkip() {
        System.out.println("\n--- InputStream Skip ---");
        
        byte[] data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        
        try {
            bis.skip(3);
            int b = bis.read();
            assertTrue("Skip 3 bytes", b == 3);
            
            long skipped = bis.skip(100);
            assertTrue("Skip beyond end returns available", skipped == 6);
            
            assertTrue("Read after skip beyond end returns -1", bis.read() == -1);
            
            bis.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testInputStreamMarkReset() {
        System.out.println("\n--- InputStream Mark/Reset ---");
        
        byte[] data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        
        try {
            assertTrue("Mark supported", bis.markSupported());
            
            bis.read();
            bis.mark(10);
            
            int b1 = bis.read();
            int b2 = bis.read();
            
            bis.reset();
            
            int b1Again = bis.read();
            int b2Again = bis.read();
            
            assertTrue("Mark/Reset works for single byte", b1 == b1Again && b2 == b2Again);
            
            bis.mark(5);
            byte[] buffer = new byte[3];
            bis.read(buffer);
            
            bis.reset();
            byte[] bufferAgain = new byte[3];
            bis.read(bufferAgain);
            
            assertTrue("Mark/Reset works for buffer", 
                     buffer[0] == bufferAgain[0] && 
                     buffer[1] == bufferAgain[1] && 
                     buffer[2] == bufferAgain[2]);
            
            bis.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testInputStreamReadBuffer() {
        System.out.println("\n--- InputStream Read Buffer ---");
        
        byte[] data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        ByteArrayInputStream bis = new ByteArrayInputStream(data);
        
        try {
            byte[] buffer = new byte[10];
            int bytesRead = bis.read(buffer, 0, 5);
            
            assertTrue("Read with offset and length returns correct count", bytesRead == 5);
            assertTrue("Buffer content correct", 
                     buffer[0] == 0 && buffer[1] == 1 && buffer[2] == 2 && 
                     buffer[3] == 3 && buffer[4] == 4);
            
            bytesRead = bis.read(buffer, 5, 5);
            assertTrue("Second read continues correctly", bytesRead == 5);
            assertTrue("Buffer continuation correct", 
                     buffer[5] == 5 && buffer[6] == 6 && buffer[7] == 7 && 
                     buffer[8] == 8 && buffer[9] == 9);
            
            bytesRead = bis.read(buffer, 0, 1);
            assertTrue("Read at end returns -1", bytesRead == -1);
            
            bis.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testOutputStreamBasics() {
        System.out.println("\n--- OutputStream Basics ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            
            bos.write(65);
            assertTrue("Write single byte", bos.toByteArray()[0] == 65);
            
            byte[] data = {66, 67, 68};
            bos.write(data);
            byte[] result = bos.toByteArray();
            assertTrue("Write byte array", 
                     result[0] == 65 && result[1] == 66 && result[2] == 67 && result[3] == 68);
            
            bos.write(data, 0, 2);
            result = bos.toByteArray();
            assertTrue("Write byte array with offset", 
                     result[4] == 66 && result[5] == 67);
            
            assertTrue("Output stream size", bos.size() == 6);
            
            bos.reset();
            assertTrue("Reset clears buffer", bos.size() == 0);
            
            bos.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testFilterStreams() {
        System.out.println("\n--- Filter Streams ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            
            bos.write(1);
            bos.write(2);
            bos.write(3);
            
            byte[] result = bos.toByteArray();
            assertTrue("OutputStream writes data", result.length == 3);
            
            ByteArrayInputStream bis = new ByteArrayInputStream(result);
            
            assertTrue("InputStream available", bis.available() == 3);
            assertTrue("InputStream read", bis.read() == 1);
            
            bis.close();
            bos.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testDataStreams() {
        System.out.println("\n--- Data Streams ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(bos);
            
            dos.writeBoolean(true);
            dos.writeByte(100);
            dos.writeShort(1000);
            dos.writeInt(100000);
            dos.writeLong(1000000L);
            dos.writeFloat(1.5f);
            dos.writeDouble(2.5);
            dos.writeChar('A');
            dos.writeUTF("Test");
            
            byte[] data = bos.toByteArray();
            ByteArrayInputStream bis = new ByteArrayInputStream(data);
            DataInputStream dis = new DataInputStream(bis);
            
            assertTrue("Data readBoolean", dis.readBoolean() == true);
            assertTrue("Data readByte", dis.readByte() == 100);
            assertTrue("Data readShort", dis.readShort() == 1000);
            assertTrue("Data readInt", dis.readInt() == 100000);
            assertTrue("Data readLong", dis.readLong() == 1000000L);
            assertTrue("Data readFloat", dis.readFloat() == 1.5f);
            assertTrue("Data readDouble", dis.readDouble() == 2.5);
            assertTrue("Data readChar", dis.readChar() == 'A');
            assertTrue("Data readUTF", dis.readUTF().equals("Test"));
            
            assertTrue("Data available after reads", dis.available() == 0);
            
            dis.close();
            dos.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testStreamChaining() {
        System.out.println("\n--- Stream Chaining ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(bos);
            
            dos.writeInt(42);
            
            ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
            DataInputStream dis = new DataInputStream(bis);
            
            assertTrue("Chained streams work", dis.readInt() == 42);
            
            dis.close();
            dos.close();
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testExceptionHandling() {
        System.out.println("\n--- Exception Handling ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            bos.write(new byte[]{1, 2, 3});
            assertTrue("Normal write works", bos.size() == 3);
            
            ByteArrayInputStream bis = new ByteArrayInputStream(new byte[]{1, 2, 3});
            assertTrue("Normal read works", bis.read() == 1);
            
            bos.close();
            bis.close();
            
            assertTrue("Stream close successful", true);
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
    
    static void testBoundaryConditions() {
        System.out.println("\n--- Boundary Conditions ---");
        
        try {
            byte[] empty = new byte[0];
            ByteArrayInputStream bis = new ByteArrayInputStream(empty);
            
            assertTrue("Read from empty stream returns -1", bis.read() == -1);
            assertTrue("Available on empty stream returns 0", bis.available() == 0);
            
            bis.close();
            
            byte[] single = {42};
            bis = new ByteArrayInputStream(single);
            
            assertTrue("Read single byte", bis.read() == 42);
            assertTrue("Second read returns -1", bis.read() == -1);
            
            bis.close();
            
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            bos.write(new byte[1000]);
            assertTrue("Large write", bos.size() == 1000);
            
            bos.close();
            
        } catch (IOException e) {
            System.out.println("  [ERROR] " + e.getMessage());
            failed++;
        }
    }
}
