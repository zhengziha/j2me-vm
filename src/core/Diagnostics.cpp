#include "Diagnostics.hpp"

namespace j2me {
namespace core {

Diagnostics& Diagnostics::getInstance() {
    static Diagnostics instance;
    return instance;
}

Diagnostics::Diagnostics() {
    startTime = std::chrono::steady_clock::now();
}

int64_t Diagnostics::nowMsNoLock() const {
    auto now = std::chrono::steady_clock::now();
    return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
}

int64_t Diagnostics::getNowMs() const {
    return nowMsNoLock();
}

void Diagnostics::onGameCanvasFlush() {
    gameCanvasFlushCount.fetch_add(1, std::memory_order_relaxed);
    lastGameCanvasFlushMs.store(nowMsNoLock(), std::memory_order_relaxed);
}

void Diagnostics::onPaintCommit() {
    paintCommitCount.fetch_add(1, std::memory_order_relaxed);
    lastPaintCommitMs.store(nowMsNoLock(), std::memory_order_relaxed);
}

void Diagnostics::onResourceNotFound(const std::string& name) {
    resourceNotFoundCount.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard<std::mutex> lock(messageMutex);
    lastResourceNotFound = name;
}

void Diagnostics::onImageDecodeFailed(const std::string& name, const std::string& headerHex) {
    imageDecodeFailedCount.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard<std::mutex> lock(messageMutex);
    lastImageDecodeFailed = name + "|" + headerHex;
}

uint64_t Diagnostics::getGameCanvasFlushCount() const {
    return gameCanvasFlushCount.load(std::memory_order_relaxed);
}

uint64_t Diagnostics::getPaintCommitCount() const {
    return paintCommitCount.load(std::memory_order_relaxed);
}

uint64_t Diagnostics::getResourceNotFoundCount() const {
    return resourceNotFoundCount.load(std::memory_order_relaxed);
}

uint64_t Diagnostics::getImageDecodeFailedCount() const {
    return imageDecodeFailedCount.load(std::memory_order_relaxed);
}

int64_t Diagnostics::getLastGameCanvasFlushMs() const {
    return lastGameCanvasFlushMs.load(std::memory_order_relaxed);
}

int64_t Diagnostics::getLastPaintCommitMs() const {
    return lastPaintCommitMs.load(std::memory_order_relaxed);
}

std::string Diagnostics::getLastResourceNotFound() const {
    std::lock_guard<std::mutex> lock(messageMutex);
    return lastResourceNotFound;
}

std::string Diagnostics::getLastImageDecodeFailed() const {
    std::lock_guard<std::mutex> lock(messageMutex);
    return lastImageDecodeFailed;
}

} // namespace core
} // namespace j2me
