package javax.microedition.media.control;

import javax.microedition.media.Control;

public interface VolumeControl extends Control {
    public static final int MUTE_SET = -1;

    int setLevel(int level);
    int getLevel();
    boolean isMuted();
    void setMute(boolean mute);
}
