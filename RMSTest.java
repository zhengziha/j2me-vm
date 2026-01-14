import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import javax.microedition.rms.*;

public class RMSTest extends Canvas {
    public RMSTest() {
        try {
            System.out.println("RMS Test starting...");
            
            // List existing record stores
            String[] stores = RecordStore.listRecordStores();
            System.out.println("Existing record stores: " + (stores != null ? stores.length : 0));
            
            // Open or create a record store
            RecordStore rs = RecordStore.openRecordStore("testStore", true);
            System.out.println("Opened record store, numRecords: " + rs.getNumRecords());
            
            // Add some records
            byte[] data1 = "Hello RMS!".getBytes();
            int id1 = rs.addRecord(data1, 0, data1.length);
            System.out.println("Added record " + id1 + ": " + data1.length + " bytes");
            
            byte[] data2 = "Second record".getBytes();
            int id2 = rs.addRecord(data2, 0, data2.length);
            System.out.println("Added record " + id2 + ": " + data2.length + " bytes");
            
            // Read back records
            byte[] read1 = rs.getRecord(id1);
            System.out.println("Read record " + id1 + ": " + new String(read1));
            
            byte[] read2 = rs.getRecord(id2);
            System.out.println("Read record " + id2 + ": " + new String(read2));
            
            // Check store info
            System.out.println("Total records: " + rs.getNumRecords());
            System.out.println("Total size: " + rs.getSize());
            System.out.println("Available: " + rs.getSizeAvailable());
            
            // Close the store
            rs.closeRecordStore();
            System.out.println("Closed record store");
            
            // Reopen and verify
            RecordStore rs2 = RecordStore.openRecordStore("testStore", false);
            System.out.println("Reopened, numRecords: " + rs2.getNumRecords());
            rs2.closeRecordStore();
            
            System.out.println("RMS Test completed successfully!");
            
        } catch (Exception e) {
            System.out.println("RMS Test failed: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    protected void paint(Graphics g) {
        g.setColor(255, 255, 255);
        g.fillRect(0, 0, getWidth(), getHeight());
        g.setColor(0, 0, 0);
        g.drawString("RMS Test - Check Console", 10, 10, Graphics.LEFT | Graphics.TOP);
    }
}
