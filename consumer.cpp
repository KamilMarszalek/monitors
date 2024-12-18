#include "consumer.h"
#include <random>
#include <fstream>
#include <iostream>

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
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "Consumed: " << this->batch << "\n";
    } else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}

void consumer::consumer_write_to_file(int state, bool saved) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        if (!saved) {
            file << "Failed to unload: " << batch << " | State: " << state << "\n";
        } else {
            file << "Successfully unloaded: " << batch << " | New state: " << state << "\n";
        }
    } else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}

void consumer::consumer_write_try_info_to_file() {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "Trying to unload: " << batch << "\n";
    } else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}