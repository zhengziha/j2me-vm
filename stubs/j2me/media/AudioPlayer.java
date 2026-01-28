package j2me.media;

import javax.microedition.media.Player;
import javax.microedition.media.MediaException;
import javax.microedition.media.Control;

public class AudioPlayer implements Player {
    private int state = UNREALIZED;
    private int loopCount = 0;
    private String contentType = "audio/unknown";
    
    public void realize() throws MediaException {
        state = REALIZED;
    }

    public void prefetch() throws MediaException {
        state = PREFETCHED;
    }

    public void start() throws MediaException {
        state = STARTED;
        // 调用本地方法播放声音
        nativeStart();
    }

    public void stop() throws MediaException {
        state = PREFETCHED;
        // 调用本地方法停止声音
        nativeStop();
    }

    public void deallocate() {
        state = REALIZED;
    }

    public void close() {
        state = CLOSED;
        nativeClose();
    }

    public long setMediaTime(long now) throws MediaException {
        return now;
    }

    public long getMediaTime() {
        return 0;
    }

    public int getState() {
        return state;
    }

    public long getDuration() {
        return 0;
    }

    public String getContentType() {
        return contentType;
    }

    public void setLoopCount(int count) {
        loopCount = count;
    }

    public Control[] getControls() {
        return new Control[] { new AudioVolumeControl() };
    }

    public Control getControl(String controlType) {
        if (controlType.indexOf("VolumeControl") != -1) {
            return new AudioVolumeControl();
        }
        return null;
    }
    
    // 本地方法
    private native void nativeStart();
    private native void nativeStop();
    private native void nativeClose();
}
