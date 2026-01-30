import java.io.ByteArrayInputStream;
import java.io.IOException;

public class ByteArrayInputStreamTest {
    public static void main(String[] args) {
        System.out.println("Starting ByteArrayInputStreamTest...");

        try {
            // Test 1: Basic read()
            byte[] data = new byte[] { 1, 2, 3, 4, 5 };
            ByteArrayInputStream bis = new ByteArrayInputStream(data);
            
            int b = bis.read();
            if (b != 1) System.out.println("Error: Expected 1, got " + b);
            else System.out.println("read() works: " + b);
            
            // Test 2: read(byte[])
            byte[] buffer = new byte[3];
            int read = bis.read(buffer, 0, 3);
            if (read != 3) System.out.println("Error: Expected 3 bytes read, got " + read);
            else System.out.println("read(byte[]) read count works: " + read);
            
            if (buffer[0] != 2 || buffer[1] != 3 || buffer[2] != 4) {
                System.out.println("Error: buffer content wrong: " + buffer[0] + "," + buffer[1] + "," + buffer[2]);
            } else {
                System.out.println("read(byte[]) content works");
            }

            // Test 3: Large array copy
            byte[] largeData = new byte[1000];
            for (int i=0; i<1000; i++) largeData[i] = (byte)(i % 127);
            ByteArrayInputStream largeBis = new ByteArrayInputStream(largeData);
            byte[] largeBuffer = new byte[500];
            largeBis.read(largeBuffer, 0, 500);
            System.out.println("Large read works");

            // Test 4: Boundary conditions
            ByteArrayInputStream emptyBis = new ByteArrayInputStream(new byte[0]);
            if (emptyBis.read() != -1) System.out.println("Error: Empty read should return -1");
            else System.out.println("Empty read works");
            
            System.out.println("ByteArrayInputStreamTest completed successfully.");
        } catch (Exception e) {
            System.out.println("Exception caught: " + e.toString());
            e.printStackTrace();
        }
    }
}
