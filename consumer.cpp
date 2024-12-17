#include "consumer.h"
#include <random>

consumer::consumer(int c, int d, int id) : c(c), d(d), id(id) {
    this->filename = "consumer_" + std::to_string(id) + ".txt";
}

consumer::~consumer() {
}

void consumer::consume() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(c, d);
    this->batch = dis(gen);
}

int consumer::get_batch() {
    return this->batch;
}

void consumer::consumer_write_consume_to_file() {
    FILE *file = fopen(filename.c_str(), "a");
    fprintf(file, "Consumed: %d\n", this->batch);
    fclose(file);
}

void consumer::consumer_write_to_file(int state, bool saved) {
    FILE *file = fopen(filename.c_str(), "a");
    if (!saved) {
        fprintf(file, "Failed to unload: %d\n", batch);
    } else {
    fprintf(file, "Succesfully unloaded: %d | New state: %d\n", batch, state);
    }
    fclose(file);
}

void consumer::consumer_write_try_info_to_file() {
    FILE *file = fopen(filename.c_str(), "a");
    fprintf(file, "Trying to unload: %d\n", batch);
    fclose(file);
}