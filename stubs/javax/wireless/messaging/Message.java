package javax.wireless.messaging;

import java.util.Date;

public interface Message {
    public String getAddress();
    public void setAddress(String addr);
    public Date getTimestamp();
}
