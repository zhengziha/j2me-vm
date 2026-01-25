package javax.wireless.messaging;

import java.util.Date;

public final class MockTextMessage implements TextMessage {
    private String address;
    private String payloadText = "";
    private Date timestamp;

    public MockTextMessage() {
        this.timestamp = new Date(System.currentTimeMillis());
    }

    public MockTextMessage(String address) {
        this.address = address;
        this.timestamp = new Date(System.currentTimeMillis());
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String addr) {
        this.address = addr;
    }

    public Date getTimestamp() {
        return timestamp;
    }

    public String getPayloadText() {
        return payloadText;
    }

    public void setPayloadText(String data) {
        this.payloadText = data == null ? "" : data;
    }
}

