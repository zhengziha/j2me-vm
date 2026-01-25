package javax.wireless.messaging;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.util.Vector;

public final class MockMessageConnection implements MessageConnection {
    private static final Vector sent = new Vector();
    private static final Vector inbox = new Vector();
    private static String lastSentAddress;
    private static String lastSentPayload;
    private static final Vector listenerConnections = new Vector();

    private String address;
    private boolean closed;
    private MessageListener listener;

    public MockMessageConnection() {
    }

    public void setDefaultAddress(String address) {
        this.address = address;
    }

    public Message newMessage(String type) {
        return newMessage(type, address);
    }

    public Message newMessage(String type, String address) {
        MockTextMessage m = new MockTextMessage(address);
        return m;
    }

    public void send(Message msg) throws IOException, InterruptedIOException {
        if (closed) throw new IOException("Connection closed");
        if (msg != null) {
            sent.addElement(msg);
        }
        String addr = address;
        if (msg != null) {
            try {
                String a = msg.getAddress();
                if (a != null && a.length() > 0) addr = a;
            } catch (Throwable t) {
            }
        }

        String payload = null;
        if (msg instanceof TextMessage) {
            try {
                payload = ((TextMessage) msg).getPayloadText();
            } catch (Throwable t) {
            }
        }

        lastSentAddress = addr;
        lastSentPayload = payload;

        MockTextMessage reply = new MockTextMessage(addr);
        reply.setAddress(addr);
        if (payload != null && payload.length() > 0) reply.setPayloadText("OK:" + payload);
        else reply.setPayloadText("OK");
        inbox.addElement(reply);

        notifyAllListeners();
    }

    public Message receive() throws IOException, InterruptedIOException {
        if (closed) throw new IOException("Connection closed");
        if (inbox.size() == 0) {
            String addr = lastSentAddress != null ? lastSentAddress : address;
            String payload = lastSentPayload;
            lastSentAddress = null;
            lastSentPayload = null;
            MockTextMessage reply = new MockTextMessage(addr);
            reply.setAddress(addr);
            if (payload != null && payload.length() > 0) reply.setPayloadText("OK:" + payload);
            else reply.setPayloadText("OK");
            return reply;
        }
        if (inbox.size() > 0) {
            Message m = (Message) inbox.elementAt(0);
            inbox.removeElementAt(0);
            return m;
        }
        return new MockTextMessage(address);
    }

    public void setMessageListener(MessageListener l) throws IOException {
        if (closed) throw new IOException("Connection closed");
        this.listener = l;
        if (l != null) {
            if (!listenerConnections.contains(this)) listenerConnections.addElement(this);
        } else {
            listenerConnections.removeElement(this);
        }
        if (l != null && inbox.size() > 0) notifyAllListeners();
    }

    public int numberOfSegments(Message msg) {
        return 1;
    }

    public void close() throws IOException {
        closed = true;
        listenerConnections.removeElement(this);
    }

    private static void notifyAllListeners() {
        if (listenerConnections.size() == 0) return;
        new Thread(new Runnable() {
            public void run() {
                for (int i = 0; i < listenerConnections.size(); i++) {
                    Object o = listenerConnections.elementAt(i);
                    if (o instanceof MockMessageConnection) {
                        MockMessageConnection c = (MockMessageConnection) o;
                        MessageListener l = c.listener;
                        if (l != null) {
                            try {
                                l.notifyIncomingMessage(c);
                            } catch (Throwable t) {
                            }
                        }
                    }
                }
            }
        }).start();
    }
}
