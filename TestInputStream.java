import java.io.*;

public class TestInputStream {
    public static void main(String[] args) {
        System.out.println("Testing InputStream operations...");
        
        // Create a byte array to test with
        byte[] testData = {65, 66, 67, 68, 69, 70}; // ABCDEF in ASCII
        
        try {
            ByteArrayInputStream bais = new ByteArrayInputStream(testData);
            
            System.out.println("Initial available: " + bais.available());
            
            // Test single byte read
            int b1 = bais.read();
            System.out.println("First byte read: " + (char)b1);
            
            System.out.println("Available after first read: " + bais.available());
            
            // Test reading into array
            byte[] buffer = new byte[3];
            int bytesRead = bais.read(buffer, 0, 3);
            System.out.println("Bytes read into buffer: " + bytesRead);
            System.out.print("Buffer contents: ");
            for (int i = 0; i < bytesRead; i++) {
                System.out.print((char)buffer[i] + " ");
            }
            System.out.println();
            
            System.out.println("Available after array read: " + bais.available());
            
            // Test skip
            long skipped = bais.skip(1);
            System.out.println("Skipped bytes: " + skipped);
            
            System.out.println("Available after skip: " + bais.available());
            
            // Test mark/reset if supported
            if (bais.markSupported()) {
                System.out.println("Mark supported, setting mark");
                bais.mark(10);
                
                int b2 = bais.read();
                System.out.println("Byte after mark: " + (char)b2);
                
                System.out.println("Available before reset: " + bais.available());
                
                bais.reset();
                System.out.println("After reset");
                
                System.out.println("Available after reset: " + bais.available());
            }
            
            // Read remaining
            while (bais.available() > 0) {
                int b = bais.read();
                if (b != -1) {
                    System.out.print((char)b);
                }
            }
            System.out.println();
            
            bais.close();
            System.out.println("Test completed successfully!");
            
        } catch (Exception e) {
            System.out.println("Error occurred: " + e.getMessage());
            e.printStackTrace();
        }
    }
}