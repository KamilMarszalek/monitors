#pragma once

#include <mutex>
#include <condition_variable>
#include "producer.h"
#include "consumer.h"

class Monitor {
    std::mutex m;
    std::condition_variable producer_cv;
    std::condition_variable consumer_cv;
    int store_state;
    int capacity;
    int producers_waiting;
    int consumers_waiting;
    int producer_count;
    int consumer_count;
    int consumer_failures;
    int producer_failures;
    int timeout;
public:
    Monitor(int capacity, int producer_count, int consumer_count, int timeout);
    ~Monitor();
    bool put(producer* prod);
    bool get(consumer* cons);
    void write_state_to_file();
    int get_state();
    bool should_producer_wait();
    bool should_consumer_wait();
    bool should_producer_signal();
    bool should_consumer_signal();
};