package javax.microedition.media.control;

import javax.microedition.media.Control;

public interface ToneControl extends Control {
    byte VERSION = 1;
    byte TEMPO = 2;
    byte RESOLUTION = 3;
    byte BLOCK_START = 4;
    byte BLOCK_END = 5;
    byte PLAY_BLOCK = 6;
    byte SET_VOLUME = 7;
    byte REPEAT = 8;
    byte C_2 = 0;
    byte C_SHARP_2 = 1;
    byte D_2 = 2;
    byte D_SHARP_2 = 3;
    byte E_2 = 4;
    byte F_2 = 5;
    byte F_SHARP_2 = 6;
    byte G_2 = 7;
    byte G_SHARP_2 = 8;
    byte A_2 = 9;
    byte A_SHARP_2 = 10;
    byte B_2 = 11;
    // ... truncated for brevity, standard MIDI notes
    byte SILENCE = -1;

    void setSequence(byte[] sequence);
}
