import java.io.ByteArrayInputStream;

public class ByteArrayInputByteSignTest {
    public static void main(String[] args) {
        System.out.println("Testing ByteArrayInputStream byte sign handling...");
        
        // Test with negative bytes (values > 127 when interpreted as unsigned)
        byte[] testData = {(byte)0x80, (byte)0xFF, (byte)0x81, (byte)0xFE, (byte)0x7F}; // -128, -1, -127, -2, 127
        System.out.println("Original byte values:");
        for (int i = 0; i < testData.length; i++) {
            System.out.println("  testData[" + i + "] = " + testData[i] + " (unsigned: " + (testData[i] & 0xFF) + ")");
        }
        
        ByteArrayInputStream bais = new ByteArrayInputStream(testData);
        
        System.out.println("\nReading from ByteArrayInputStream:");
        int value;
        int index = 0;
        while ((value = bais.read()) != -1) {
            System.out.println("  read() returned: " + value + " (should be unsigned byte value)");
            
            // Compare with manual conversion
            int expected = testData[index] & 0xFF;
            if (value == expected) {
                System.out.println("    ✓ Correct: got " + value + ", expected " + expected);
            } else {
                System.out.println("    ✗ Error: got " + value + ", expected " + expected);
            }
            index++;
        }
        
        System.out.println("\nTesting the difference between signed and unsigned interpretation:");
        byte negativeByte = (byte)0xFF; // This is -1 as a signed byte
        int unsignedValue = negativeByte & 0xFF; // This is 255 as unsigned
        
        System.out.println("  Signed byte 0xFF = " + negativeByte);
        System.out.println("  Unsigned value of same byte = " + unsignedValue);
        
        // Test with ByteArrayInputStream
        byte[] singleNegative = {(byte)0xFF};
        ByteArrayInputStream bais2 = new ByteArrayInputStream(singleNegative);
        int result = bais2.read();
        System.out.println("  ByteArrayInputStream.read() returned: " + result);
        System.out.println("  Should match unsigned value: " + (result == unsignedValue ? "✓" : "✗"));
        
        System.out.println("\nTest completed.");
    }
}