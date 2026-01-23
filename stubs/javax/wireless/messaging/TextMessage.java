package javax.wireless.messaging;

public interface TextMessage extends Message {
    public String getPayloadText();
    public void setPayloadText(String data);
}
