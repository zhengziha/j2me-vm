package j2me.media;

import javax.microedition.media.control.VolumeControl;

public class DummyVolumeControl implements VolumeControl {
    private int level = 50;
    private boolean muted = false;

    public int setLevel(int level) {
        if (level < 0) level = 0;
        if (level > 100) level = 100;
        this.level = level;
        return level;
    }

    public int getLevel() {
        return level;
    }

    public boolean isMuted() {
        return muted;
    }

    public void setMute(boolean mute) {
        this.muted = mute;
    }
}
