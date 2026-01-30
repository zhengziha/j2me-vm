import java.io.InputStream;
import java.io.ByteArrayInputStream;

public class TestResourceAsStream {
    public static void main(String[] args) {
        System.out.println("Testing Class.getResourceAsStream()...");
        
        try {
            // Test with a resource that should exist
            InputStream is = TestResourceAsStream.class.getResourceAsStream("/META-INF/MANIFEST.MF");
            
            System.out.println("InputStream obtained: " + is);
            System.out.println("Is ByteArrayInputStream: " + (is instanceof ByteArrayInputStream));
            
            if (is != null) {
                // Read and print the resource content
                System.out.println("\nResource content:");
                int b;
                while ((b = is.read()) != -1) {
                    System.out.print((char)b);
                }
                is.close();
            } else {
                System.out.println("Resource not found!");
            }
            
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
        
        System.out.println("\nTest completed.");
    }
}
