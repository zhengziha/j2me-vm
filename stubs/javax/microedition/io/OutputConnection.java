package javax.microedition.io;

import java.io.DataOutputStream;
import java.io.OutputStream;
import java.io.IOException;

public interface OutputConnection extends Connection {
    public DataOutputStream openDataOutputStream() throws IOException;
    public OutputStream openOutputStream() throws IOException;
}
