#include "monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>

Monitor::Monitor(int capacity, int producer_count, int consumer_count, int timeout) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&producer_cond, NULL);
    pthread_cond_init(&consumer_cond, NULL);
    this->store_state = 0;
    this->producers_waiting = 0;
    this->consumers_waiting = 0;
    this->consumer_failures = 0;
    this->producer_failures = 0;
    this->producer_count = producer_count;
    this->consumer_count = consumer_count;
    this->capacity = capacity;
    this->timeout = timeout;
    write_state_to_file();
}

Monitor::~Monitor() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&producer_cond);
    pthread_cond_destroy(&consumer_cond);
}

void Monitor::enter() {
    pthread_mutex_lock(&mutex);
}

void Monitor::leave() {
    pthread_mutex_unlock(&mutex);
}

bool Monitor::put (producer* prod) {
    sleep(timeout);
    this->enter();
    while (should_producer_wait()) {
        producer_wait();
    }
    sleep(timeout);
    if (store_state + prod->get_batch() > capacity) {
        // sleep(timeout);
        producer_failures++;
        if (should_consumer_broadcast()){
            consumer_broadcast();
            producer_wait();
        }
        this->leave();
        sleep(timeout);
        return false;
    }
    store_state += prod->get_batch();
    write_state_to_file();
    sleep(timeout);
    if (should_consumer_signal()) {
        consumer_broadcast();
    }
    producer_failures = 0;
    this->leave();
    return true;
}

bool Monitor::get (consumer* cons) {
    sleep(timeout);
    this->enter();
    while (should_consumer_wait()) {
        consumer_wait();
    }
    if (store_state - cons->get_batch() < 0) {
        // sleep(timeout);
        consumer_failures++;
        if (should_producer_broadcast()) {
            producer_broadcast();
            consumer_wait();
        }
        this->leave();
        sleep(timeout);
        return false;
    }
    sleep(timeout);
    store_state -= cons->get_batch();
    write_state_to_file();
    sleep(timeout);
    if (should_producer_signal()) {
        producer_broadcast();
    }
    consumer_failures = 0;
    this->leave();
    return true;
}

void Monitor::write_state_to_file() {
    FILE *file = fopen("warehouse.txt", "w");
    fprintf(file, "%d\n", store_state);
    fflush(file);
    fclose(file);
}

int Monitor::get_state() {
    return store_state;
}


bool Monitor::should_producer_wait() {
    // return store_state > capacity / 2;
    return store_state > capacity / 2 && consumer_failures < consumer_count;
}

bool Monitor::should_consumer_wait() {
    // return store_state <= capacity / 2;
    return store_state <= capacity / 2 && producer_failures < producer_count;
}

bool Monitor::should_producer_signal() {
    return store_state <= capacity / 2 || 
        producer_failures == producer_count;
}

bool Monitor::should_consumer_signal() {
    return store_state > capacity / 2 ||
        consumer_failures == consumer_count;
}

void Monitor::producer_wait() {
    producers_waiting++;
    pthread_cond_wait(&this->producer_cond, &this->mutex);
    producers_waiting--;
}

void Monitor::consumer_wait() {
    consumers_waiting++;
    pthread_cond_wait(&this->consumer_cond, &this->mutex);
    consumers_waiting--;
}

void Monitor::producer_signal() {
    pthread_cond_signal(&this->producer_cond);
}

void Monitor::consumer_signal() {
    pthread_cond_signal(&this->consumer_cond);
}

void Monitor::consumer_broadcast() {
    pthread_cond_broadcast(&this->consumer_cond);
}

void Monitor::producer_broadcast() {
    pthread_cond_broadcast(&this->producer_cond);
}

bool Monitor::should_producer_broadcast() {
    return consumer_failures == consumer_count && producers_waiting == producer_count;
}

bool Monitor::should_consumer_broadcast() {
    return producer_failures == producer_count && consumers_waiting == consumer_count;
}

