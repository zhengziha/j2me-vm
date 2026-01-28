import javax.microedition.midlet.MIDlet;
import javax.microedition.lcdui.Display;
import java.io.RandomAccessFile;

public class RandomAccessTestMIDlet extends MIDlet {
    
    public RandomAccessTestMIDlet() {
        System.out.println("RandomAccessTestMIDlet Constructor");
    }
    
    protected void startApp() {
        System.out.println("RandomAccessFileTest Started");
        
        try {
            RandomAccessFile raf = new RandomAccessFile("tests/test_res.bin", "r");
            
            System.out.println("File opened successfully");
            
            long length = raf.length();
            System.out.println("File length: " + length);
            
            long pos = raf.getFilePointer();
            System.out.println("Initial position: " + pos);
            
            byte[] buffer = new byte[10];
            int bytesRead = raf.read(buffer);
            System.out.println("Read " + bytesRead + " bytes from start");
            System.out.print("Content: ");
            for (int i = 0; i < bytesRead; i++) {
                System.out.print((char)buffer[i]);
            }
            System.out.println();
            
            pos = raf.getFilePointer();
            System.out.println("Position after first read: " + pos);
            
            raf.seek(0);
            System.out.println("Seeked to position 0");
            
            pos = raf.getFilePointer();
            System.out.println("Current position: " + pos);
            
            raf.seek(25);
            System.out.println("Seeked to position 25");
            
            bytesRead = raf.read(buffer);
            System.out.println("Read " + bytesRead + " bytes from position 25");
            System.out.print("Content: ");
            for (int i = 0; i < bytesRead; i++) {
                System.out.print((char)buffer[i]);
            }
            System.out.println();
            
            raf.seek(length - 10);
            System.out.println("Seeked to position " + (length - 10));
            
            bytesRead = raf.read(buffer);
            System.out.println("Read " + bytesRead + " bytes from end");
            System.out.print("Content: ");
            for (int i = 0; i < bytesRead; i++) {
                System.out.print((char)buffer[i]);
            }
            System.out.println();
            
            raf.close();
            System.out.println("File closed");
            
            System.out.println("RandomAccessFileTest Completed Successfully!");
            
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    protected void pauseApp() {
        System.out.println("pauseApp called");
    }
    
    protected void destroyApp(boolean unconditional) {
        System.out.println("destroyApp called");
    }
}
