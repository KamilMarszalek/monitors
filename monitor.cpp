#include "monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>

Monitor::Monitor(int capacity, int producer_count, int consumer_count, int timeout) {
    sem_init(&mutex, 0, 1);
    sem_init(&producer_cond, 0, 0);
    sem_init(&consumer_cond, 0, 0);
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
    sem_destroy(&mutex);
    sem_destroy(&producer_cond);
    sem_destroy(&consumer_cond);
}

void Monitor::enter() {
    sem_wait(&mutex);
}

void Monitor::leave() {
    sem_post(&mutex);
}

bool Monitor::put (producer* prod) {
    this->enter();
    prod->producer_write_try_info_to_file();
    while (should_producer_wait()) {
        sleep(timeout);
        producer_wait();
    }
    sleep(timeout);
    if (store_state + prod->get_batch() > capacity) {
        producer_failures++;
        if (should_consumer_signal()){
            consumer_signal();
        } else {
            producer_signal();
        }
        sleep(timeout);
        prod->producer_write_to_file(get_state(), false);
        sleep(timeout);
        return false;
    }
    sleep(timeout);
    store_state += prod->get_batch();
    write_state_to_file();
    prod->producer_write_to_file(get_state(), true);
    sleep(timeout);
    if (should_consumer_signal()) {
        consumer_signal();
    } else {
        producer_signal();
    }
    producer_failures = 0;
    return true;
}

bool Monitor::get (consumer* cons) {
    this->enter();
    cons->consumer_write_try_info_to_file();
    while (should_consumer_wait()) {
        sleep(timeout);
        consumer_wait();
    }
    if (store_state - cons->get_batch() < 0) {
        consumer_failures++;
        if (should_producer_signal()) {
            producer_signal();
        } else {
            consumer_signal();
        }
        sleep(timeout);
        cons->consumer_write_to_file(get_state(), false);
        sleep(timeout);
        return false;
    }
    sleep(timeout);
    store_state -= cons->get_batch();
    write_state_to_file();
    cons->consumer_write_to_file(get_state(), true);
    sleep(timeout);
    if (should_producer_signal()) {
        producer_signal();
    } else {
        consumer_signal();
    }
    consumer_failures = 0;
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
    return store_state > capacity / 2;
}

bool Monitor::should_consumer_wait() {
    // return store_state <= capacity / 2;
    return store_state <= capacity / 2;
}

bool Monitor::should_producer_signal() {
    return store_state <= capacity / 2;
}

bool Monitor::should_consumer_signal() {
    return store_state > capacity / 2;
}

void Monitor::producer_wait() {
    producers_waiting++;
    leave();
    sem_wait(&producer_cond);
    producers_waiting--;
}

void Monitor::consumer_wait() {
    consumers_waiting++;
    leave();
    sem_wait(&consumer_cond);
    consumers_waiting--;
}

void Monitor::producer_signal() {
    if (producers_waiting > 0)
        sem_post(&producer_cond);
    else leave();
}

void Monitor::consumer_signal() {
    if (consumers_waiting > 0)
        sem_post(&consumer_cond);
    else leave();
}

// void Monitor::consumer_broadcast() {
//     pthread_cond_broadcast(&this->consumer_cond);
// }

// void Monitor::producer_broadcast() {
//     pthread_cond_broadcast(&this->producer_cond);
// }

// bool Monitor::should_producer_broadcast() {
//     return consumer_failures == consumer_count && producers_waiting == producer_count;
// }

// bool Monitor::should_consumer_broadcast() {
//     return producer_failures == producer_count && consumers_waiting == consumer_count;
// }

