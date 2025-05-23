// Anteater Electric Racing, 2025

#include <cstdint>
#include <cstddef>
#include "vehicle/telemetry.h"
#include <cassert>
#include <malloc.h>
#include <mutex>

template <typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t size) :
        buf(std::unique_ptr<T[]>(new T[size])),
        max_size(size), head(0), tail(0), full(false) {}

    void put(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        buf[head] = item;
        if (full) {
            tail = (tail + 1) % max_size;
        }
        head = (head + 1) % max_size;
        full = (head == tail);
    }

    T get() {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(!empty());
        T item = buf[tail];
        full = false;
        tail = (tail + 1) % max_size;
        return item;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        head = tail;
        full = false;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return (!full && (head == tail));
    }

    bool full() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return full;
    }

    size_t capacity() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return max_size;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t size = max_size;
        if (!full) {
            if (head >= tail) {
                size = head - tail;
            } else {
                size = max_size + head - tail;
            }
        }
        return size;
    }

private:
    std::mutex mutex_;
    std::unique_ptr<T[]> buf;
    size_t head = 0;
    size_t tail = 0;
    const size_t max_size;
    bool full = false;
};