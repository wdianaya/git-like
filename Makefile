CC = gcc
LIBS = -lssl -lcrypto

TARGET = app

OBJS = main.o repository.o utils.o object.o index.o commit.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o

repository.o: repository.c
	$(CC) -c repository.c -o repository.o

utils.o: utils.c
	$(CC) -c utils.c -o utils.o

object.o: object.c
	$(CC) -c object.c -o object.o

index.o: index.c
	$(CC) -c index.c -o index.o

commit.o: commit.c
	$(CC) -c commit.c -o commit.o

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean