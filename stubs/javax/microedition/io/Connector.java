package javax.microedition.io;

import java.io.IOException;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class Connector {
    public static final int READ = 1;
    public static final int WRITE = 2;
    public static final int READ_WRITE = 3;

    public static Connection open(String name) throws IOException {
        return null;
    }

    public static Connection open(String name, int mode) throws IOException {
        return null;
    }

    public static Connection open(String name, int mode, boolean timeouts) throws IOException {
        return null;
    }

    public static DataInputStream openDataInputStream(String name) throws IOException {
        return null;
    }

    public static DataOutputStream openDataOutputStream(String name) throws IOException {
        return null;
    }

    public static InputStream openInputStream(String name) throws IOException {
        return null;
    }

    public static OutputStream openOutputStream(String name) throws IOException {
        return null;
    }
}
