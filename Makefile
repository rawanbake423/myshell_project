CC = gcc
CFLAGS = -Wall
TARGET = myshell
SRC = myshell.c utility.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

