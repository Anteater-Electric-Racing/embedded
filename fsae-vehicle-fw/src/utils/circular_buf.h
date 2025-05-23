// Anteater Electric Racing, 2025

#include <cstdint>
#include <cstddef>
#include "vehicle/telemetry.h"
#include <cassert>
#include <malloc.h>
#include <mutex>
#include <memory>

template <typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t size) :
        buf(std::make_unique<T[]>(size)),
        max_size(size), head(0), tail(0), is_full(false) {}

    void put(const T& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        buf[head] = data;
        if (is_full) {
            tail = (tail + 1) % max_size;
        }
        head = (head + 1) % max_size;
        is_full = (head == tail);
    }

    bool get(T& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!empty()) {
            data = buf[tail];
            is_full = false;
            tail = (tail + 1) % max_size;
            return true;
        }
        return false;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        head = tail;
        is_full = false;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return (!is_full && (head == tail));
    }

    bool full() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return is_full;
    }

    size_t capacity() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return max_size;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t size = max_size;
        if (!is_full) {
            if (head >= tail) {
                size = head - tail;
            } else {
                size = max_size + head - tail;
            }
        }
        return size;
    }

    void printBuffer() const {
        std::lock_guard<std::mutex> lock(mutex_);
        Serial.println("CircularBuffer contents:");
        for (size_t i = 0; i < max_size; ++i) {
            if (i == head) Serial.print("[H]");
            if (i == tail) Serial.print("[T]");

            Serial.print("Index ");
            Serial.print(i);
            Serial.print(": ");
            
            printTelemetryData(buf[i]);
            Serial.println();
        }
        Serial.println();
    }

private:
std::unique_ptr<T[]> buf;
const size_t max_size;
size_t head;
size_t tail;
bool is_full = false;
mutable std::mutex mutex_;
};