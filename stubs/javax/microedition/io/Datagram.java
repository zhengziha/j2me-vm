package javax.microedition.io;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

public interface Datagram extends DataInput, DataOutput {
    public String getAddress();
    public byte[] getData();
    public int getLength();
    public int getOffset();
    public void setAddress(String addr) throws IOException;
    public void setAddress(Datagram reference);
    public void setData(byte[] buffer, int offset, int len);
    public void setLength(int len);
    public void reset();
}
