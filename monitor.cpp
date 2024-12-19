#include "monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <iostream>
#include <fstream>

Monitor::Monitor(int capacity, int producer_count, int consumer_count, int timeout) : capacity(capacity), producer_count(producer_count), consumer_count(consumer_count), timeout(timeout) {
    store_state = 0;
    producers_waiting = 0;
    consumers_waiting = 0;
    consumer_failures = 0;
    producer_failures = 0;
    write_state_to_file();
}

Monitor::~Monitor() {
}

bool Monitor::put(producer* prod) {
    std::unique_lock<std::mutex> lk(m);
    while (should_producer_wait()) {
        producers_waiting++;
        producer_cv.wait(lk);
        producers_waiting--;
    }
    prod->producer_write_try_info_to_file();
    sleep(timeout);
    if (store_state + prod->get_batch() > capacity) {
        producer_failures++;
        if (should_consumer_signal()){
            consumer_cv.notify_one();
        } else {
            producer_cv.notify_one();
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
    if (should_consumer_signal()) {
        consumer_cv.notify_one();
    } else {
        producer_cv.notify_one();
    }
    producer_failures = 0;
    return true;
}

bool Monitor::get(consumer* cons) {
    std::unique_lock<std::mutex> lk(m);
    while (should_consumer_wait()) {
        consumers_waiting++;
        consumer_cv.wait(lk);
        consumers_waiting--;
    }
    cons->consumer_write_try_info_to_file();
    if (store_state - cons->get_batch() < 0) {
        consumer_failures++;
        if (should_producer_signal()) {
            producer_cv.notify_one();
        } else {
            consumer_cv.notify_one();
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
    if (should_producer_signal()) {
        producer_cv.notify_one();
    } else {
        consumer_cv.notify_one();
    }
    consumer_failures = 0;
    return true;
}

void Monitor::write_state_to_file() {
    std::ofstream file("warehouse.txt");
    if (file.is_open()) {
        file << store_state << "\n";
        file.flush();
    } else {
        std::cerr << "Error opening file for writing: warehouse.txt" << std::endl;
    }
}

int Monitor::get_state() {
    return store_state;
}


bool Monitor::should_producer_wait() {
    return (store_state > capacity / 2) && (producers_waiting + consumers_waiting < producer_count + consumer_count - 1)
        && (consumer_failures <= 2 * consumer_count);
}

bool Monitor::should_consumer_wait() {
    return (store_state <= capacity / 2) && (producers_waiting + consumers_waiting < producer_count + consumer_count - 1)
        && (producer_failures <= 2 * producer_count);
}

bool Monitor::should_producer_signal() {
    return store_state <= capacity / 2 || consumer_failures > 2 * consumer_count;
}

bool Monitor::should_consumer_signal() {
    return store_state > capacity / 2 || producer_failures > 2 * producer_count;
}
