package javax.wireless.messaging;

import javax.microedition.io.Connection;
import java.io.IOException;
import java.io.InterruptedIOException;

public interface MessageConnection extends Connection {
    public static final String TEXT_MESSAGE = "text";
    public static final String BINARY_MESSAGE = "binary";
    public static final String MULTIPART_MESSAGE = "multipart";

    public Message newMessage(String type);
    public Message newMessage(String type, String address);
    public void send(Message msg) throws IOException, InterruptedIOException;
    public Message receive() throws IOException, InterruptedIOException;
    public void setMessageListener(MessageListener l) throws IOException;
    public int numberOfSegments(Message msg);
}
