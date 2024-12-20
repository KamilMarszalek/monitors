#include "monitor.h"
#include "producer.h"
#include "consumer.h"
#include <thread>
#include <iostream>
#include <unistd.h>

struct ThreadArgs {
    producer* prod;
    consumer* cons;
    Monitor* monitor;
};

void producer_thread(ThreadArgs* args) {
    producer *prod = args->prod;
    Monitor *monitor = args->monitor;
    bool status = true;

    while (true) {
        if (status) {
            prod->produce();
            prod->producer_write_produced_to_file();
            status = false;
        }
        status = monitor->put(prod);
        sleep(1);
    }
}

void consumer_thread(ThreadArgs* args) {
    consumer *cons = args->cons;
    Monitor *monitor = args->monitor;
    bool status = true;

    while (true) {
        if (status) {
            cons->consume();
            cons->consumer_write_consume_to_file();
            status = false;
        }
        status = monitor->get(cons);
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 9) {
        std::cout << "Usage: " << argv[0] << " <number of producers> <number of consumers> <begin> <end> <begin> <end> <capacity> <timeout>\n";
        return 1;
    }
    
    system("rm -f producer_*.txt consumer_*.txt warehouse.txt");
    int number_of_producers = atoi(argv[1]);
    int number_of_consumers = atoi(argv[2]);
    int a = atoi(argv[3]);
    int b = atoi(argv[4]);
    int c = atoi(argv[5]);
    int d = atoi(argv[6]);
    int capacity = atoi(argv[7]);
    int timeout = atoi(argv[8]);
    Monitor* monitor = new Monitor(capacity, number_of_producers, number_of_consumers, timeout);
    sleep(timeout);
    producer *producers[number_of_producers];
    consumer *consumers[number_of_consumers];
    std::thread producer_threads[number_of_producers];
    std::thread consumer_threads[number_of_consumers];
    ThreadArgs producer_args[number_of_producers];
    ThreadArgs consumer_args[number_of_consumers];

    for (int i = 0; i < number_of_producers; i++) {
        producers[i] = new producer(a, b, i);
        producer_args[i] = {producers[i], nullptr, monitor};
        producer_threads[i] = std::thread(producer_thread, &producer_args[i]);
    }

    for (int i = 0; i < number_of_consumers; i++) {
        consumers[i] = new consumer(c, d, i);
        consumer_args[i] = {nullptr, consumers[i], monitor};
        consumer_threads[i] = std::thread(consumer_thread, &consumer_args[i]);
    }

    for (int i = 0; i < number_of_producers; i++) {
        producer_threads[i].join();
    }

    for (int i = 0; i < number_of_consumers; i++) {
        consumer_threads[i].join();
    }

    delete monitor;
    return 0;
}