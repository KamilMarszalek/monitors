#pragma once

#include <semaphore.h>
#include <pthread.h>

class Monitor {
    pthread_mutex_t mutex;
    pthread_cond_t producer;
    pthread_cond_t consumer;
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
    void enter();
    void leave();
    bool put(int item);
    bool get(int item);
    void write_state_to_file();
    int get_state();
    bool should_producer_wait();
    bool should_consumer_wait();
    bool should_producer_signal();
    bool should_consumer_signal();
    void producer_wait();
    void consumer_wait();
    void producer_signal();
    void consumer_signal();
    void consumer_broadcast();
    void producer_broadcast();
};