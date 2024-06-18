
CC = gcc
CFLAGS = -shared -fPIC
LDFLAGS = -L. -lexample
LDLIBS = -ldl

TARGET_LIB = libexample.so
TARGET_LOADER = dynamic_loader
TARGET_MAIN = main

SRC_LIB = example.c
SRC_LOADER = dynamic_loader.c
SRC_MAIN = main.c

all: $(TARGET_LIB) $(TARGET_LOADER) $(TARGET_MAIN)

$(TARGET_LIB): $(SRC_LIB)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_LOADER): $(SRC_LOADER)
	$(CC) -o $@ $^ $(LDLIBS)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CC) -o $@ $^ $(LDFLAGS)

run: all
	LD_LIBRARY_PATH=. ./$(TARGET_LOADER) $(TARGET_LIB)

clean:
	rm -f $(TARGET_LIB) $(TARGET_LOADER) $(TARGET_MAIN)
