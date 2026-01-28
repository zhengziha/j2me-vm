package j2me.media;

import javax.microedition.media.control.VolumeControl;

public class AudioVolumeControl implements VolumeControl {
    private int volume = 80; // 默认音量80%
    
    public int getLevel() {
        return volume;
    }
    
    public int setLevel(int level) {
        // 确保音量在0-100之间
        if (level < 0) {
            volume = 0;
        } else if (level > 100) {
            volume = 100;
        } else {
            volume = level;
        }
        
        // 调用本地方法设置音量
        nativeSetVolume(volume);
        return volume;
    }
    
    public boolean isMuted() {
        return volume == 0;
    }
    
    public void setMute(boolean mute) {
        if (mute) {
            volume = 0;
        } else {
            volume = 80; // 恢复默认音量
        }
        
        nativeSetVolume(volume);
    }
    
    // 本地方法
    private native void nativeSetVolume(int volume);
}
