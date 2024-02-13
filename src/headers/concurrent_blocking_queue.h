#ifndef ILASH_CONCURRENT_BLOCKING_QUEUE_H
#define ILASH_CONCURRENT_BLOCKING_QUEUE_H

#endif //ILASH_CONCURRENT_BLOCKING_QUEUE_H

#include <cstddef>
#include <deque>
#include <mutex>
#include <condition_variable>

// A simple bounced, blocking, synchronized queue
// From https://morestina.net/blog/1400/minimalistic-blocking-bounded-queue-for-c

template<typename T>
class concurrent_blocking_queue {
    std::deque<T> content;
    size_t capacity;

    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

public:
    explicit concurrent_blocking_queue(size_t capacity): capacity(capacity) {}

    concurrent_blocking_queue(const concurrent_blocking_queue &) = delete;
    concurrent_blocking_queue(concurrent_blocking_queue &&) = delete;
    concurrent_blocking_queue &operator = (const concurrent_blocking_queue &) = delete;
    concurrent_blocking_queue &operator = (concurrent_blocking_queue &&) = delete;

    void push(T &&item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            not_full.wait(lk, [this]() { return content.size() < capacity; });
            content.push_back(std::move(item));
        }
        not_empty.notify_one();
    }

    bool try_push(T &&item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            if (content.size() == capacity)
                return false;
            content.push_back(std::move(item));
        }
        not_empty.notify_one();
        return true;
    }

    void pop(T &item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            not_empty.wait(lk, [this]() { return !content.empty(); });
            item = std::move(content.front());
            content.pop_front();
        }
        not_full.notify_one();
    }

    bool try_pop(T &item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            if (content.empty())
                return false;
            item = std::move(content.front());
            content.pop_front();
        }
        not_full.notify_one();
        return true;
    }
};