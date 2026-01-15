package javax.microedition.io;

import java.io.DataInputStream;
import java.io.IOException;

public interface ContentConnection extends InputConnection {
    String getEncoding();
    String getType();
    String getHeaderField(String name);
    String getHeaderField(int n);
    String getHeaderFieldKey(int n);
    long getLength();
}
