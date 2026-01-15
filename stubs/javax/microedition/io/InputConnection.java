package javax.microedition.io;

import java.io.DataInputStream;
import java.io.IOException;

public interface InputConnection extends Connection {
    DataInputStream openDataInputStream() throws IOException;
    java.io.InputStream openInputStream() throws IOException;
}
