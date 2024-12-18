#include "producer.h"
#include <random>
#include <fstream>
#include <iostream>

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
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "Produced: " << this->batch << "\n";
    } else {
        std::cerr << "Error opening file for writing: " << filename << "\n";
    }
}

void producer::producer_write_to_file(int state, bool saved) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        if (!saved) {
            file << "Failed to load: " << batch << " | State: " << state << "\n";
        } else {
            file << "Successfully loaded: " << batch << " | New state: " << state << "\n";
        }
    } else {
        std::cerr << "Error opening file for writing: " << filename << "\n";
    }
}

void producer::producer_write_try_info_to_file() {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "Trying to load: " << batch << "\n";
    } else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}