package j2me.media;

import javax.microedition.media.Player;
import javax.microedition.media.MediaException;
import javax.microedition.media.Control;

public class DummyPlayer implements Player {
    public void realize() throws MediaException {
        // No-op
    }

    public void prefetch() throws MediaException {
        // No-op
    }

    public void start() throws MediaException {
        // No-op
    }

    public void stop() throws MediaException {
        // No-op
    }

    public void deallocate() {
        // No-op
    }

    public void close() {
        // No-op
    }

    public long setMediaTime(long now) throws MediaException {
        return now;
    }

    public long getMediaTime() {
        return 0;
    }

    public int getState() {
        return Player.REALIZED;
    }

    public long getDuration() {
        return 0;
    }

    public String getContentType() {
        return "audio/unknown";
    }

    public void setLoopCount(int count) {
        // No-op
    }

    public Control[] getControls() {
        return new Control[] { new DummyVolumeControl() };
    }

    public Control getControl(String controlType) {
        if (controlType.indexOf("VolumeControl") != -1) {
            return new DummyVolumeControl();
        }
        return null;
    }
}
