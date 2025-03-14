#pragma once

#include <semaphore.h>
#include <pthread.h>
#include "producer.h"
#include "consumer.h"

class Monitor {
    sem_t mutex;
    sem_t producer_cond;
    sem_t consumer_cond;
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
    bool put(producer* prod);
    bool get(consumer* cons);
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
};