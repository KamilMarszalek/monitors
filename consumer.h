#pragma once
#include <string>
class consumer {
    int c; // begin of range
    int d; // end of range
    int batch;
    int id;
    std::string filename;
public:
    consumer(int c, int d, int id);
    ~consumer();
    void consume();
    int get_batch();
    void consumer_write_consume_to_file();
    void consumer_write_to_file(int state, bool saved);
    void consumer_write_try_info_to_file();
};