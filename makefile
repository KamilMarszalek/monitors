CC = g++
CFLAGS = -Wall -g -pthread
LDFLAGS = -lm -pthread

all: main

main: main.o consumer.o producer.o monitor.o
	$(CC) $(CFLAGS) -o main main.o consumer.o producer.o monitor.o $(LDFLAGS)

main.o: main.cpp consumer.h producer.h monitor.h
	$(CC) $(CFLAGS) -c main.cpp -o main.o

monitor.o: monitor.cpp monitor.h
	$(CC) $(CFLAGS) -c monitor.cpp -o monitor.o

consumer.o: consumer.cpp consumer.h
	$(CC) $(CFLAGS) -c consumer.cpp -o consumer.o

producer.o: producer.cpp producer.h
	$(CC) $(CFLAGS) -c producer.cpp -o producer.o

clean:
	rm -f *.o main *.txt