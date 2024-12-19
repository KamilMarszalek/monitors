#include "monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>

Monitor::Monitor(int capacity, int producer_count, int consumer_count, int timeout) : capacity(capacity), producer_count(producer_count), consumer_count(consumer_count), timeout(timeout) {
    sem_init(&mutex, 0, 1);
    sem_init(&producer_cond, 0, 0);
    sem_init(&consumer_cond, 0, 0);
    store_state = 0;
    producers_waiting = 0;
    consumers_waiting = 0;
    consumer_failures = 0;
    producer_failures = 0;
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
    enter();
    if (should_producer_wait()) {
        producer_wait();
    }
    prod->producer_write_try_info_to_file();
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
        return false;
    }
    store_state += prod->get_batch();
    write_state_to_file();
    prod->producer_write_to_file(get_state(), true);
    if (should_consumer_signal()) {
        consumer_signal();
    } else {
        producer_signal();
    }
    producer_failures = 0;
    return true;
}

bool Monitor::get (consumer* cons) {
    enter();
    if (should_consumer_wait()) {
        consumer_wait();
    }
    cons->consumer_write_try_info_to_file();
    sleep(timeout);
    if (store_state - cons->get_batch() < 0) {
        consumer_failures++;
        if (should_producer_signal()) {
            producer_signal();
        } else {
            consumer_signal();
        }
        sleep(timeout);
        cons->consumer_write_to_file(get_state(), false);
        return false;
    }
    store_state -= cons->get_batch();
    write_state_to_file();
    cons->consumer_write_to_file(get_state(), true);
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
    return store_state > capacity / 2 && producers_waiting + consumers_waiting < producer_count + consumer_count - 1;
}

bool Monitor::should_consumer_wait() {
    return store_state <= capacity / 2 && producers_waiting + consumers_waiting < producer_count + consumer_count - 1;
}

bool Monitor::should_producer_signal() {
    return store_state <= capacity / 2 || consumer_failures > 2 * consumer_count;
}

bool Monitor::should_consumer_signal() {
    return store_state > capacity / 2 || producer_failures > 2 * producer_count;
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