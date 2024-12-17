#pragma once
#include <string>
class producer {
    int a; // begin of range
    int b; // end of range
    int batch;
    int id;
    std::string filename;
public:
    producer(int a, int b, int id);
    ~producer();
    void produce();
    int get_batch();
    void producer_write_produced_to_file();
    void producer_write_to_file(int state, bool saved);
    void producer_write_try_info_to_file();
};