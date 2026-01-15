package javax.microedition.io;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public interface HttpConnection extends ContentConnection {
    static final int GET = 1;
    static final int POST = 2;
    static final int HEAD = 3;

    String getURL();
    String getProtocol();
    String getHost();
    String getFile();
    String getRef();
    String getQuery();
    int getPort();
    String getRequestMethod();
    void setRequestMethod(String method) throws IOException;
    String getRequestProperty(String key);
    void setRequestProperty(String key, String value) throws IOException;
    int getResponseCode() throws IOException;
    String getResponseMessage() throws IOException;
    long getExpiration();
    long getDate();
    long getLastModified();
    String getHeaderField(String name);
    String getHeaderField(int n);
    String getHeaderFieldKey(int n);
    DataInputStream openDataInputStream() throws IOException;
    InputStream openInputStream() throws IOException;
    OutputStream openOutputStream() throws IOException;
}
