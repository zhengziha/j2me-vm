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
        return open(name, READ_WRITE);
    }

    public static Connection open(String name, int mode) throws IOException {
        return open(name, mode, false);
    }

    public static Connection open(String name, int mode, boolean timeouts) throws IOException {
        // Native delegation
        // In a real implementation, this would parse the URL and find a protocol handler
        // For now, let's just assume we have a native method that returns a Connection object
        // or a handle. But Connection is an interface.
        // The VM should create an instance of a class implementing the specific Connection interface.
        return openNative(name, mode, timeouts);
    }

    public static DataInputStream openDataInputStream(String name) throws IOException {
        Connection c = open(name, READ);
        if (!(c instanceof InputConnection)) {
            try {
                c.close();
            } catch (IOException e) {}
            throw new IllegalArgumentException("Not an input connection");
        }
        return ((InputConnection)c).openDataInputStream();
    }

    public static DataOutputStream openDataOutputStream(String name) throws IOException {
        Connection c = open(name, WRITE);
        if (!(c instanceof OutputConnection)) {
            try {
                c.close();
            } catch (IOException e) {}
            throw new IllegalArgumentException("Not an output connection");
        }
        return ((OutputConnection)c).openDataOutputStream();
    }

    public static InputStream openInputStream(String name) throws IOException {
        Connection c = open(name, READ);
        if (!(c instanceof InputConnection)) {
            try {
                c.close();
            } catch (IOException e) {}
            throw new IllegalArgumentException("Not an input connection");
        }
        return ((InputConnection)c).openInputStream();
    }

    public static OutputStream openOutputStream(String name) throws IOException {
        Connection c = open(name, WRITE);
        if (!(c instanceof OutputConnection)) {
            try {
                c.close();
            } catch (IOException e) {}
            throw new IllegalArgumentException("Not an output connection");
        }
        return ((OutputConnection)c).openOutputStream();
    }

    private static native Connection openNative(String name, int mode, boolean timeouts) throws IOException;
}
