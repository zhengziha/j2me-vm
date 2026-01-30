import java.io.*;
import java.util.Vector;

public class IOTest {
    public static void main(String[] args) {
        System.out.println("=== I/O Operations Test ===");
        
        testByteArrayInputStream();
        testByteArrayOutputStream();
        testDataInputStream();
        testDataOutputStream();
        testStringReader();
        testStringWriter();
        
        System.out.println("=== All I/O Tests Completed ===");
    }
    
    static void testByteArrayInputStream() {
        System.out.println("\n--- ByteArrayInputStream Test ---");
        
        byte[] data = {72, 101, 108, 108, 111, 32, 74, 50, 77, 69};
        
        try {
            ByteArrayInputStream bis = new ByteArrayInputStream(data);
            
            System.out.println("Available bytes: " + bis.available());
            
            if (bis.available() == 10) {
                System.out.println("ByteArrayInputStream available: PASSED");
            } else {
                System.out.println("ByteArrayInputStream available: FAILED");
            }
            
            System.out.println("Reading bytes:");
            int b;
            while ((b = bis.read()) != -1) {
                System.out.print(" " + b + "(" + (char)b + ")");
            }
            System.out.println();
            
            bis.reset();
            byte[] buffer = new byte[5];
            int bytesRead = bis.read(buffer);
            
            System.out.println("Read " + bytesRead + " bytes");
            System.out.println("Buffer content: " + new String(buffer, 0, bytesRead));
            
            if (bytesRead == 5 && new String(buffer, 0, bytesRead).equals("Hello")) {
                System.out.println("ByteArrayInputStream read: PASSED");
            } else {
                System.out.println("ByteArrayInputStream read: FAILED");
            }
            
            bis.close();
            System.out.println("ByteArrayInputStream closed successfully");
            
        } catch (IOException e) {
            System.out.println("ByteArrayInputStream test FAILED: " + e.getMessage());
        }
    }
    
    static void testByteArrayOutputStream() {
        System.out.println("\n--- ByteArrayOutputStream Test ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            
            String str1 = "Hello";
            String str2 = " ";
            String str3 = "J2ME";
            
            bos.write(str1.getBytes());
            bos.write(str2.getBytes());
            bos.write(str3.getBytes());
            
            byte[] result = bos.toByteArray();
            String resultStr = new String(result);
            
            System.out.println("Written content: " + resultStr);
            System.out.println("Size: " + bos.size());
            
            if (resultStr.equals("Hello J2ME") && bos.size() == 9) {
                System.out.println("ByteArrayOutputStream write: PASSED");
            } else {
                System.out.println("ByteArrayOutputStream write: FAILED");
            }
            
            bos.write(33);
            result = bos.toByteArray();
            resultStr = new String(result);
            
            System.out.println("After writing 33 (!): " + resultStr);
            
            if (resultStr.equals("Hello J2ME!")) {
                System.out.println("ByteArrayOutputStream single byte: PASSED");
            } else {
                System.out.println("ByteArrayOutputStream single byte: FAILED");
            }
            
            bos.reset();
            System.out.println("After reset, size: " + bos.size());
            
            if (bos.size() == 0) {
                System.out.println("ByteArrayOutputStream reset: PASSED");
            } else {
                System.out.println("ByteArrayOutputStream reset: FAILED");
            }
            
            bos.close();
            System.out.println("ByteArrayOutputStream closed successfully");
            
        } catch (IOException e) {
            System.out.println("ByteArrayOutputStream test FAILED: " + e.getMessage());
        }
    }
    
    static void testDataInputStream() {
        System.out.println("\n--- DataInputStream Test ---");
        
        try {
            byte[] data = new byte[20];
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(bos);
            
            dos.writeByte(127);
            dos.writeShort(32767);
            dos.writeInt(2147483647);
            dos.writeLong(9223372036854775807L);
            dos.writeFloat(3.14f);
            dos.writeDouble(2.71828);
            dos.writeUTF("J2ME");
            
            byte[] testData = bos.toByteArray();
            ByteArrayInputStream bis = new ByteArrayInputStream(testData);
            DataInputStream dis = new DataInputStream(bis);
            
            byte b = dis.readByte();
            short s = dis.readShort();
            int i = dis.readInt();
            long l = dis.readLong();
            float f = dis.readFloat();
            double d = dis.readDouble();
            String str = dis.readUTF();
            
            System.out.println("Read byte: " + b);
            System.out.println("Read short: " + s);
            System.out.println("Read int: " + i);
            System.out.println("Read long: " + l);
            System.out.println("Read float: " + f);
            System.out.println("Read double: " + d);
            System.out.println("Read UTF: " + str);
            
            if (b == 127 && s == 32767 && i == 2147483647 && 
                l == 9223372036854775807L && f == 3.14f && 
                d == 2.71828 && str.equals("J2ME")) {
                System.out.println("DataInputStream read: PASSED");
            } else {
                System.out.println("DataInputStream read: FAILED");
            }
            
            dis.close();
            dos.close();
            
        } catch (IOException e) {
            System.out.println("DataInputStream test FAILED: " + e.getMessage());
        }
    }
    
    static void testDataOutputStream() {
        System.out.println("\n--- DataOutputStream Test ---");
        
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(bos);
            
            System.out.println("Writing primitive types...");
            dos.writeBoolean(true);
            dos.writeByte(100);
            dos.writeShort(1000);
            dos.writeInt(100000);
            dos.writeLong(1000000L);
            dos.writeFloat(1.5f);
            dos.writeDouble(2.5);
            dos.writeChars("Test");
            
            byte[] result = bos.toByteArray();
            System.out.println("Written " + result.length + " bytes");
            
            ByteArrayInputStream bis = new ByteArrayInputStream(result);
            DataInputStream dis = new DataInputStream(bis);
            
            boolean bool = dis.readBoolean();
            byte b = dis.readByte();
            short s = dis.readShort();
            int i = dis.readInt();
            long l = dis.readLong();
            float f = dis.readFloat();
            double d = dis.readDouble();
            
            System.out.println("Read back: " + bool + ", " + b + ", " + s + ", " + 
                              i + ", " + l + ", " + f + ", " + d);
            
            if (bool && b == 100 && s == 1000 && i == 100000 && 
                l == 1000000L && f == 1.5f && d == 2.5) {
                System.out.println("DataOutputStream write: PASSED");
            } else {
                System.out.println("DataOutputStream write: FAILED");
            }
            
            int size = dos.size();
            System.out.println("DataOutputStream size: " + size);
            
            dos.close();
            dis.close();
            
        } catch (IOException e) {
            System.out.println("DataOutputStream test FAILED: " + e.getMessage());
        }
    }
    
    static void testStringReader() {
        System.out.println("\n--- StringReader Test ---");
        
        try {
            String str = "Hello J2ME World";
            StringReader reader = new StringReader(str);
            
            System.out.println("String: " + str);
            
            int ch;
            System.out.println("Reading characters:");
            while ((ch = reader.read()) != -1) {
                System.out.print((char)ch);
            }
            System.out.println();
            
            reader.reset();
            
            char[] buffer = new char[5];
            int charsRead = reader.read(buffer, 0, 5);
            
            System.out.println("Read " + charsRead + " characters: " + new String(buffer, 0, charsRead));
            
            if (charsRead == 5 && new String(buffer, 0, charsRead).equals("Hello")) {
                System.out.println("StringReader read: PASSED");
            } else {
                System.out.println("StringReader read: FAILED");
            }
            
            reader.skip(1);
            charsRead = reader.read(buffer, 0, 3);
            System.out.println("After skip, read " + charsRead + " characters: " + new String(buffer, 0, charsRead));
            
            if (charsRead == 3 && new String(buffer, 0, charsRead).equals("J2M")) {
                System.out.println("StringReader skip: PASSED");
            } else {
                System.out.println("StringReader skip: FAILED");
            }
            
            reader.close();
            System.out.println("StringReader closed successfully");
            
        } catch (IOException e) {
            System.out.println("StringReader test FAILED: " + e.getMessage());
        }
    }
    
    static void testStringWriter() {
        System.out.println("\n--- StringWriter Test ---");
        
        try {
            StringWriter writer = new StringWriter();
            
            System.out.println("Writing to StringWriter...");
            writer.write("Hello");
            writer.write(" ");
            writer.write("J2ME");
            writer.write(" ");
            writer.write("World");
            
            String result = writer.toString();
            System.out.println("Written content: " + result);
            
            if (result.equals("Hello J2ME World")) {
                System.out.println("StringWriter write: PASSED");
            } else {
                System.out.println("StringWriter write: FAILED");
            }
            
            StringBuffer buffer = writer.getBuffer();
            System.out.println("Buffer length: " + buffer.length());
            System.out.println("Buffer content: " + buffer.toString());
            
            if (buffer.length() == 16) {
                System.out.println("StringWriter buffer: PASSED");
            } else {
                System.out.println("StringWriter buffer: FAILED");
            }
            
            writer.write("!");
            result = writer.toString();
            System.out.println("After appending '!': " + result);
            
            if (result.equals("Hello J2ME World!")) {
                System.out.println("StringWriter append: PASSED");
            } else {
                System.out.println("StringWriter append: FAILED");
            }
            
            writer.close();
            System.out.println("StringWriter closed successfully");
            
        } catch (IOException e) {
            System.out.println("StringWriter test FAILED: " + e.getMessage());
        }
    }
}
