CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 
TARGET_DIR := .
TARGET := $(TARGET_DIR)/main.exe
SOURCES := main.cpp src/process.cpp src/fcfs.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(TARGET_DIR)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
