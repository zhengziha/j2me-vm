#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <string>

namespace j2me {
namespace core {

class Diagnostics {
public:
    static Diagnostics& getInstance();

    void onGameCanvasFlush();
    void onPaintCommit();
    void onResourceNotFound(const std::string& name);
    void onImageDecodeFailed(const std::string& name, const std::string& headerHex);

    uint64_t getGameCanvasFlushCount() const;
    uint64_t getPaintCommitCount() const;
    uint64_t getResourceNotFoundCount() const;
    uint64_t getImageDecodeFailedCount() const;

    int64_t getLastGameCanvasFlushMs() const;
    int64_t getLastPaintCommitMs() const;
    int64_t getNowMs() const;
    std::string getLastResourceNotFound() const;
    std::string getLastImageDecodeFailed() const;

private:
    Diagnostics();

    int64_t nowMsNoLock() const;

    std::chrono::steady_clock::time_point startTime{};
    std::atomic<uint64_t> gameCanvasFlushCount{0};
    std::atomic<uint64_t> paintCommitCount{0};
    std::atomic<int64_t> lastGameCanvasFlushMs{0};
    std::atomic<int64_t> lastPaintCommitMs{0};
    std::atomic<uint64_t> resourceNotFoundCount{0};
    std::atomic<uint64_t> imageDecodeFailedCount{0};
    mutable std::mutex messageMutex;
    std::string lastResourceNotFound;
    std::string lastImageDecodeFailed;
};

} // namespace core
} // namespace j2me
