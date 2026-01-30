#include "MIDIPlayer.hpp"
#include "Logger.hpp"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <map>

namespace j2me {
namespace core {

// 大端序读取辅助函数
inline static uint16_t readBigEndian16(const uint8_t* data) {
    return (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
}

inline static uint32_t readBigEndian32(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8) |
           static_cast<uint32_t>(data[3]);
}

// ==================== MIDIFile ====================

MIDIFile::MIDIFile()
    : format(0), numTracks(0), ticksPerBeat(480), ticksPerFrame(0), valid(false)
{
}

MIDIFile::~MIDIFile() {
}

uint32_t MIDIFile::readVarLen(const uint8_t*& data, size_t& size) {
    uint32_t value = 0;
    uint8_t byte;
    do {
        if (size == 0) return 0;
        byte = *data++;
        size--;
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    return value;
}

bool MIDIFile::parseTrack(const uint8_t* data, size_t size, MIDITrack& track) {
    const uint8_t* end = data + size;
    uint8_t lastStatus = 0;

    while (data < end) {
        MIDIEvent event;
        event.deltaTime = readVarLen(data, size);

        if (size < 1) break;

        uint8_t firstByte = *data;

        // Running Status
        if (firstByte < 0x80) {
            if (lastStatus == 0) return false;
            event.status = lastStatus;
        } else {
            event.status = firstByte;
            lastStatus = event.status;
            data++;
            size--;
        }

        uint8_t highNibble = event.status >> 4;
        uint8_t lowNibble = event.status & 0x0F;

        switch (highNibble) {
            case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
                event.data.push_back(event.status);
                if (size < 1) break;
                event.data.push_back(*data++);
                size--;
                if (size < 1) break;
                event.data.push_back(*data++);
                size--;
                break;

            case 0xC: case 0xD:
                event.data.push_back(event.status);
                if (size < 1) break;
                event.data.push_back(*data++);
                size--;
                break;

            case 0xF:
                if (lowNibble == 0x0 || lowNibble == 0x7) {
                    uint32_t len = readVarLen(data, size);
                    if (len > size) break;
                    data += len;
                    size -= len;
                } else if (lowNibble == 0xF) {
                    if (size < 1) break;
                    uint8_t metaType = *data++;
                    size--;
                    uint32_t len = readVarLen(data, size);
                    if (len > size) break;
                    if (metaType == 0x2F) goto track_end;
                    data += len;
                    size -= len;
                }
                break;

            default:
                return false;
        }

        track.events.push_back(event);
    }

track_end:
    track.ticks = 0;
    for (const auto& evt : track.events) {
        track.ticks += evt.deltaTime;
    }
    return true;
}

bool MIDIFile::loadFromMemory(const std::vector<uint8_t>& data) {
    if (data.size() < 14) {
        return false;
    }

    const uint8_t* ptr = data.data();
    size_t size = data.size();

    // Check MThd
    if (std::memcmp(ptr, "MThd", 4) != 0) {
        return false;
    }
    ptr += 4;
    size -= 4;

    uint32_t headerLen = readBigEndian32(ptr);
    ptr += 4;
    size -= 4;

    format = readBigEndian16(ptr);
    ptr += 2;
    numTracks = readBigEndian16(ptr);
    ptr += 2;
    ticksPerBeat = readBigEndian16(ptr);
    ptr += 2;
    size -= 6;

    tracks.clear();
    for (int i = 0; i < numTracks; i++) {
        if (size < 8) break;

        if (std::memcmp(ptr, "MTrk", 4) != 0) {
            return false;
        }
        ptr += 4;
        size -= 4;

        uint32_t trackLen = readBigEndian32(ptr);
        ptr += 4;
        size -= 4;

        if (trackLen > size) {
            return false;
        }

        MIDITrack track;
        if (!parseTrack(ptr, trackLen, track)) {
            return false;
        }

        tracks.push_back(track);
        ptr += trackLen;
        size -= trackLen;
    }

    valid = true;
    return true;
}

// ==================== FMSynthesizer ====================

FMSynthesizer::FMSynthesizer()
    : sampleRate(44100), totalSamples(0)
{
    reset();
}

FMSynthesizer::~FMSynthesizer() {
}

void FMSynthesizer::init(int rate) {
    sampleRate = rate;
    reset();
}

void FMSynthesizer::reset() {
    for (int i = 0; i < 16; i++) {
        channels[i].reset();
    }
    totalSamples = 0;
}

void FMSynthesizer::noteOn(int channel, int note, int velocity) {
    if (channel < 0 || channel >= 16) return;
    channels[channel].startNote(note, velocity, totalSamples);
}

void FMSynthesizer::noteOff(int channel, int note) {
    if (channel < 0 || channel >= 16) return;

    if (channels[channel].note == note && channels[channel].active) {
        if (channels[channel].sustain == 0) {
            channels[channel].active = false;
        }
    }
}

void FMSynthesizer::programChange(int channel, int program) {
    if (channel < 0 || channel >= 16) return;
    channels[channel].program = program;
}

void FMSynthesizer::controlChange(int channel, int controller, int value) {
    if (channel < 0 || channel >= 16) return;

    switch (controller) {
        case 0: // Bank Select
            break;
        case 1: // Modulation Wheel
            channels[channel].modulation = value;
            break;
        case 7: // Volume
            channels[channel].volume = value;
            break;
        case 10: // Pan
            channels[channel].pan = value;
            break;
        case 11: // Expression
            channels[channel].expression = value;
            break;
        case 64: // Sustain Pedal
            if (channels[channel].sustain > 0 && value == 0) {
                for (int i = 0; i < 16; i++) {
                    if (i != channel && channels[i].active) {
                        channels[i].active = false;
                    }
                }
            }
            channels[channel].sustain = value;
            break;
        default:
            break;
    }
}

void FMSynthesizer::pitchBend(int channel, int value) {
    if (channel < 0 || channel >= 16) return;
    channels[channel].pitchBend = value;
}

int FMSynthesizer::getNoteFrequency(int note) {
    // A4 (note 69) = 440 Hz
    return static_cast<int>(440.0 * std::pow(2.0, (note - 69) / 12.0));
}

void FMSynthesizer::generateVoice(int16_t* output, int samples, Channel& channel) {
    int frequency = getNoteFrequency(channel.note);

    // 正弦波合成 - 大幅增加振幅
    double amplitude = (channel.velocity / 127.0) * 25000.0;
    amplitude = amplitude * channel.volume / 100.0;
    amplitude = amplitude * channel.expression / 127.0;

    // 相位增量（弧度/样本）
    double phaseInc = (frequency * 2.0 * M_PI) / sampleRate;

    for (int i = 0; i < samples; i++) {
        // 计算音符开始后的总时间（样本数）
        int64_t noteTime = channel.samplesPlayed + i;

        // 生成正弦波
        double rawSample = std::sin(channel.phase);

        // 包络 - 移除 attack phase
        int decaySamples = sampleRate / 2;      // 500ms
        int sustainLevel = 50;

        double env = 1.0;
        if (noteTime < decaySamples) {
            double t = (double)noteTime / decaySamples;
            env = 1.0 - (1.0 - sustainLevel / 100.0) * t;
        } else {
            env = sustainLevel / 100.0;
        }

        double sample = rawSample * amplitude * env;

        // 防削波
        if (sample > 30000.0) sample = 30000.0;
        if (sample < -30000.0) sample = -30000.0;

        // 单声道输出
        output[i] += (int16_t)sample;

        // 累加相位，保持在 [0, 2π) 范围内
        channel.phase += phaseInc;
        while (channel.phase >= 2.0 * M_PI) {
            channel.phase -= 2.0 * M_PI;
        }
    }

    channel.samplesPlayed += samples;
}

void FMSynthesizer::generate(int16_t* output, int samples) {
    // 清空输出
    std::memset(output, 0, samples * sizeof(int16_t));

    // 处理所有活动的通道
    int activeCount = 0;
    for (int ch = 0; ch < 16; ch++) {
        if (!channels[ch].active) continue;
        activeCount++;

        generateVoice(output, samples, channels[ch]);
    }

    // 调试输出已禁用 - 音频合成工作正常

    totalSamples += samples;
}

// ==================== MIDIPlayer ====================

MIDIPlayer::MIDIPlayer()
    : playing(false)
    , paused(false)
    , looping(false)
    , volume(100)
    , currentTick(0)
    , sampleRate(44100)
    , tempo(500000)
{
}

MIDIPlayer::~MIDIPlayer() {
}

bool MIDIPlayer::loadFromMemory(const std::vector<uint8_t>& data) {
    if (!midiFile.loadFromMemory(data)) {
        return false;
    }

    sampleRate = 44100;
    updateTempo();

    // 初始化合成器
    synthesizer.init(sampleRate);

    // 初始化音轨事件索引和时间计数器
    trackEventIndex.clear();
    trackEventTime.clear();
    nextEventTime.clear();
    trackEventIndex.resize(midiFile.getNumTracks(), 0);
    trackEventTime.resize(midiFile.getNumTracks());
    nextEventTime.resize(midiFile.getNumTracks(), 0);

    // 计算每个音轨中事件的绝对时间（累计 ticks）
    for (int i = 0; i < midiFile.getNumTracks(); i++) {
        const auto& events = midiFile.getTracks()[i].events;
        uint32_t absTime = 0;
        for (const auto& event : events) {
            absTime += event.deltaTime;
            trackEventTime[i].push_back(absTime);
        }
        // 设置第一个事件的时间
        if (!trackEventTime[i].empty()) {
            nextEventTime[i] = trackEventTime[i][0];
        }
    }

    std::stringstream ss;
    ss << "MIDI header: format=" << midiFile.getFormat() << ", tracks=" << midiFile.getNumTracks()
       << ", ticksPerBeat=" << midiFile.getTicksPerBeat();
    LOG_INFO(ss.str().c_str());

    return true;
}

void MIDIPlayer::updateTempo() {
    ticksPerMs = static_cast<double>(midiFile.getTicksPerBeat()) / (tempo / 1000000.0);
}

bool MIDIPlayer::play() {
    if (!midiFile.isValid()) {
        return false;
    }

    currentTick = 0;
    playing = true;
    paused = false;

    return true;
}

void MIDIPlayer::stop() {
    playing = false;
    paused = false;
    currentTick = 0;

    // 重置所有音轨索引和时间
    for (size_t i = 0; i < trackEventIndex.size(); i++) {
        trackEventIndex[i] = 0;
        if (i < nextEventTime.size()) {
            if (!trackEventTime[i].empty()) {
                nextEventTime[i] = trackEventTime[i][0];
            }
        }
    }

    synthesizer.reset();
}

void MIDIPlayer::pause() {
    paused = true;
}

void MIDIPlayer::resume() {
    paused = false;
}

bool MIDIPlayer::isPlaying() const {
    return playing && !paused;
}

bool MIDIPlayer::isPaused() const {
    return paused;
}

void MIDIPlayer::setVolume(int vol) {
    volume = std::max(0, std::min(100, vol));
}

bool MIDIPlayer::getAudioData(int16_t* output, int samples) {
    if (!playing || paused || !midiFile.isValid()) {
        return false;
    }

    // 清空输出
    std::memset(output, 0, samples * sizeof(int16_t));

    // 计算需要处理的 ticks 数量
    double ticksPerSample = ticksPerMs / 1000.0;
    uint64_t ticksToProcess = static_cast<uint64_t>(samples * ticksPerSample);

    // 处理所有音轨的事件
    while (ticksToProcess > 0) {
        bool eventProcessed = false;

        for (int track = 0; track < midiFile.getNumTracks(); track++) {
            const auto& tracks = midiFile.getTracks();
            const auto& events = tracks[track].events;

            // 检查是否有事件需要处理
            while (trackEventIndex[track] < events.size() &&
                   nextEventTime[track] <= currentTick) {

                const auto& event = events[trackEventIndex[track]];

                // 处理 MIDI 事件
                uint8_t status = event.status;
                uint8_t channel = status & 0x0F;
                uint8_t type = status >> 4;

                switch (type) {
                    case 0x8: // Note Off
                        if (event.data.size() >= 2) {
                            synthesizer.noteOff(channel, event.data[1]);
                        }
                        break;

                    case 0x9: // Note On
                        if (event.data.size() >= 3) {
                            if (event.data[2] > 0) {
                                synthesizer.noteOn(channel, event.data[1], event.data[2]);
                            } else {
                                synthesizer.noteOff(channel, event.data[1]);
                            }
                        }
                        break;

                    case 0xB: // Control Change
                        if (event.data.size() >= 3) {
                            synthesizer.controlChange(channel, event.data[1], event.data[2]);
                        }
                        break;

                    case 0xC: // Program Change
                        if (event.data.size() >= 2) {
                            synthesizer.programChange(channel, event.data[1]);
                        }
                        break;

                    case 0xE: // Pitch Bend
                        if (event.data.size() >= 3) {
                            int value = (event.data[2] << 7) | event.data[1];
                            synthesizer.pitchBend(channel, value);
                        }
                        break;

                    case 0xF: // System/Meta events - skip
                        break;
                }

                // 移动到下一个事件
                trackEventIndex[track]++;
                if (trackEventIndex[track] < trackEventTime[track].size()) {
                    nextEventTime[track] = trackEventTime[track][trackEventIndex[track]];
                }

                eventProcessed = true;
            }
        }

        // 前进时间
        currentTick++;

        // 如果所有事件都处理完了，检查是否循环
        bool allTracksFinished = true;
        for (int track = 0; track < midiFile.getNumTracks(); track++) {
            if (trackEventIndex[track] < midiFile.getTracks()[track].events.size()) {
                allTracksFinished = false;
                break;
            }
        }

        if (allTracksFinished) {
            if (looping) {
                // 重新开始
                currentTick = 0;
                for (int i = 0; i < midiFile.getNumTracks(); i++) {
                    trackEventIndex[i] = 0;
                    if (!trackEventTime[i].empty()) {
                        nextEventTime[i] = trackEventTime[i][0];
                    }
                }
                synthesizer.reset();
            } else {
                playing = false;
                break;
            }
        }

        ticksToProcess--;
    }

    // 生成音频
    synthesizer.generate(output, samples);

    // 应用音量
    if (volume < 100) {
        float volFactor = volume / 100.0f;
        for (int i = 0; i < samples; i++) {
            output[i] = static_cast<int16_t>(output[i] * volFactor);
        }
    }

    return true;
}

int MIDIPlayer::getDurationMs() const {
    if (!midiFile.isValid()) {
        return 0;
    }

    // 估算时长（基于第一个音轨）
    const auto& track = midiFile.getTracks()[0];
    double totalTicks = track.ticks;
    double durationMs = totalTicks / ticksPerMs;

    return static_cast<int>(durationMs);
}

} // namespace core
} // namespace j2me
