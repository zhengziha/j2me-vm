#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace j2me {
namespace core {

// MIDI 事件类型
enum MIDIEventType {
    MIDI_NOTE_OFF = 0x80,
    MIDI_NOTE_ON = 0x90,
    MIDI_POLY_PRESSURE = 0xA0,
    MIDI_CONTROL_CHANGE = 0xB0,
    MIDI_PROGRAM_CHANGE = 0xC0,
    MIDI_PITCH_BEND = 0xE0
};

// MIDI 事件
struct MIDIEvent {
    uint32_t deltaTime;  // Delta 时间（ticks）
    uint8_t status;       // 状态字节
    std::vector<uint8_t> data;

    MIDIEvent() : deltaTime(0), status(0) {}
};

// MIDI 音轨
struct MIDITrack {
    std::vector<MIDIEvent> events;
    uint32_t ticks;  // 音轨总时长（ticks）
};

// MIDI 文件
class MIDIFile {
public:
    MIDIFile();
    ~MIDIFile();

    // 从内存加载 MIDI 文件
    bool loadFromMemory(const std::vector<uint8_t>& data);

    // 获取格式信息
    int getFormat() const { return format; }
    int getNumTracks() const { return numTracks; }
    int getTicksPerBeat() const { return ticksPerBeat; }
    int getTicksPerFrame() const { return ticksPerFrame; } // SMPTE

    // 获取音轨
    const std::vector<MIDITrack>& getTracks() const { return tracks; }

    bool isValid() const { return valid; }

private:
    int format;           // 0 = 单音轨, 1 = 多音轨, 2 = 多音轨序列
    int numTracks;
    int ticksPerBeat;     // 每四分音符的 ticks
    int ticksPerFrame;    // SMPTE 帧格式
    bool valid;

    std::vector<MIDITrack> tracks;

    // 读取变长数量
    static uint32_t readVarLen(const uint8_t*& data, size_t& size);

    // 解析 MIDI Track chunk
    bool parseTrack(const uint8_t* data, size_t size, MIDITrack& track);
};

// FM 音色合成器（简单实现）
class FMSynthesizer {
public:
    FMSynthesizer();
    ~FMSynthesizer();

    // 初始化
    void init(int sampleRate);

    // 重置所有通道
    void reset();

    // 处理 MIDI 事件
    void noteOn(int channel, int note, int velocity);
    void noteOff(int channel, int note);
    void programChange(int channel, int program);
    void controlChange(int channel, int controller, int value);
    void pitchBend(int channel, int value);

    // 生成音频样本
    void generate(int16_t* output, int samples);

    // 设置采样率
    void setSampleRate(int rate) { sampleRate = rate; reset(); }

private:
    // 通道状态
    struct Channel {
        bool active;
        int note;
        int velocity;
        int program;
        int pitchBend;
        int volume;
        int pan;
        int expression;

        // FM 合成参数
        int modulation;
        int sustain;

        // 语音状态（每个通道独立）
        double phase;  // 相位（使用 double 防止精度丢失）
        int64_t noteStartTime;  // 音符开始时间（样本数）
        int samplesPlayed;      // 此音符已播放的样本数

        Channel() : active(false), note(0), velocity(0), program(0),
                    pitchBend(0x2000), volume(100), pan(64), expression(127),
                    modulation(0), sustain(0), phase(0.0), noteStartTime(0), samplesPlayed(0) {}

        void reset() {
            active = false;
            note = 0;
            velocity = 0;
            phase = 0.0;
            noteStartTime = 0;
            samplesPlayed = 0;
        }

        void startNote(int n, int v, int64_t startTime) {
            active = true;
            note = n;
            velocity = v;
            phase = 0.0;
            noteStartTime = startTime;
            samplesPlayed = 0;
        }
    };

    Channel channels[16];
    int sampleRate;
    int64_t totalSamples;  // 总样本数计数器

    // 音符频率（Hz）
    static int getNoteFrequency(int note);

    // 生成波形（使用通道状态）
    void generateVoice(int16_t* output, int samples, Channel& channel);
};

// MIDI 播放器
class MIDIPlayer {
public:
    MIDIPlayer();
    ~MIDIPlayer();

    // 从内存加载 MIDI 文件
    bool loadFromMemory(const std::vector<uint8_t>& data);

    // 播放控制
    bool play();
    void stop();
    void pause();
    void resume();
    bool isPlaying() const;
    bool isPaused() const;

    // 设置循环
    void setLoop(bool loop) { looping = loop; }

    // 获取音频数据（供回调使用）
    bool getAudioData(int16_t* output, int samples);

    // 设置音量 (0-100)
    void setVolume(int vol);
    int getVolume() const { return volume; }

    // 获取信息
    bool isValid() const { return midiFile.isValid(); }
    int getNumTracks() const { return midiFile.getNumTracks(); }
    int getDurationMs() const; // 估算时长（毫秒）

private:
    MIDIFile midiFile;
    FMSynthesizer synthesizer;

    // 播放状态
    bool playing;
    bool paused;
    bool looping;
    int volume; // 0-100

    // 时间管理
    uint64_t currentTick;
    int sampleRate;

    // 速度 (TEMPO)
    int tempo;  // microseconds per quarter note
    double ticksPerMs;

    // 每个音轨当前的事件索引
    std::vector<size_t> trackEventIndex;
    std::vector<uint32_t> nextEventTime;    // 下一个事件的绝对时间（ticks）
    std::vector<std::vector<uint32_t>> trackEventTime;  // 每个音轨的事件绝对时间列表

    // 节奏映射
    void updateTempo();
};

} // namespace core
} // namespace j2me
