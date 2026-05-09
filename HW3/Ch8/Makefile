CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11

TARGET  = banker
SRC     = banker.c

.PHONY: all clean demo demo_request

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Textbook example (5 processes, 3 resources) — safe state check only
demo: $(TARGET)
	./$(TARGET) \
		-n 5 -m 3 \
		-avail  3 3 2 \
		-alloc  0 1 0  2 0 0  3 0 2  2 1 1  0 0 2 \
		-max    7 5 3  3 2 2  9 0 2  2 2 2  4 3 3

# Same example + P1 requests (1, 0, 2)
demo_request: $(TARGET)
	./$(TARGET) \
		-n 5 -m 3 \
		-avail  3 3 2 \
		-alloc  0 1 0  2 0 0  3 0 2  2 1 1  0 0 2 \
		-max    7 5 3  3 2 2  9 0 2  2 2 2  4 3 3 \
		-request 1  1 0 2

clean:
	rm -f $(TARGET)
