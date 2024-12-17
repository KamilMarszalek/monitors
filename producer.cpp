#include "producer.h"
#include <random>

producer::producer(int a, int b, int id) : a(a), b(b), id(id) {
    this->filename = "producer_" + std::to_string(id) + ".txt";
}

producer::~producer() {
}

void producer::produce() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(a, b);
    this->batch = dis(gen);
}

int producer::get_batch() {
    return this->batch;
}

void producer::producer_write_produced_to_file() {
    FILE *file = fopen(filename.c_str(), "a");
    fprintf(file, "Produced: %d\n", this->batch);
    fclose(file);
}

void producer::producer_write_to_file(int state, bool saved) {
    FILE *file = fopen(filename.c_str(), "a");
    if (!saved) {
        fprintf(file, "Failed to load: %d\n", batch);
    } else{
    fprintf(file, "Succesfully loaded: %d | New state: %d\n", batch, state);
    }
    fclose(file);
}

void producer::producer_write_try_info_to_file() {
    FILE *file = fopen(filename.c_str(), "a");
    fprintf(file, "Trying to load: %d\n", batch);
    fclose(file);
}