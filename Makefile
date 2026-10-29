
CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
TARGET = sws
SOURCE = src/sws.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
