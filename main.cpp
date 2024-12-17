#include "monitor.h"
#include "producer.h"
#include "consumer.h"
#include <pthread.h>
#include <iostream>
#include <unistd.h>

struct ThreadArgs {
    producer* prod;
    consumer* cons;
    Monitor* monitor;
};

void* producer_thread(void *arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    producer *prod = args->prod;
    Monitor *monitor = args->monitor;
    bool status = true;

    while (true) {
        if (status) {
            prod->produce();
            prod->producer_write_produced_to_file();
            status = false;
        }
        prod->producer_write_try_info_to_file();
        status = monitor->put(prod->get_batch());
        prod->producer_write_to_file(monitor->get_state(), status);
        sleep(1);
    }
    return NULL;
}

void* consumer_thread(void *arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    consumer *cons = args->cons;
    Monitor *monitor = args->monitor;
    bool status = true;

    while (true) {
        if (status) {
            cons->consume();
            cons->consumer_write_consume_to_file();
            status = false;
        }
        cons->consumer_write_try_info_to_file();
        status = monitor->get(cons->get_batch());
        cons->consumer_write_to_file(monitor->get_state(), status);
        sleep(1);
    }
    return NULL;
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
    pthread_t producer_threads[number_of_producers];
    pthread_t consumer_threads[number_of_consumers];
    ThreadArgs producer_args[number_of_producers];
    ThreadArgs consumer_args[number_of_consumers];

    for (int i = 0; i < number_of_producers; i++) {
        producers[i] = new producer(a, b, i);
        producer_args[i] = {producers[i], nullptr, monitor};
        pthread_create(&producer_threads[i], NULL, producer_thread, &producer_args[i]);
    }
    for (int i = 0; i < number_of_consumers; i++) {
        consumers[i] = new consumer(c, d, i);
        consumer_args[i] = {nullptr, consumers[i], monitor};
        pthread_create(&consumer_threads[i], NULL, consumer_thread, &consumer_args[i]);
    }

    for (int i = 0; i < number_of_producers; i++) {
        pthread_join(producer_threads[i], NULL);
    }
    for (int i = 0; i < number_of_consumers; i++) {
        pthread_join(consumer_threads[i], NULL);
    }
    delete monitor;
    return 0;
}