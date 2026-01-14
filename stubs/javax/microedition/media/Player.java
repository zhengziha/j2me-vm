package javax.microedition.media;

public interface Player {
    public static final int UNREALIZED = 0;
    public static final int REALIZED = 100;
    public static final int PREFETCHED = 200;
    public static final int STARTED = 300;
    public static final int CLOSED = 400;

    void realize() throws MediaException;
    void prefetch() throws MediaException;
    void start() throws MediaException;
    void stop() throws MediaException;
    void deallocate();
    void close();
    long setMediaTime(long now) throws MediaException;
    long getMediaTime();
    int getState();
    long getDuration();
    String getContentType();
    void setLoopCount(int count);
    Control[] getControls();
    Control getControl(String controlType);
}
