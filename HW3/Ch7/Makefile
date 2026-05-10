CC = gcc
CFLAGS = -pthread -Wall -std=c99
TARGET = producer_consumer
SRC = producer_consumer.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lpthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean
