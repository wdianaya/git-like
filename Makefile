CC = gcc
CFLAGS = -I./include 
LIBS = -lssl -lcrypto

TARGET = app

OBJS = build/main.o build/repository.o build/utils.o build/object.o build/index.o build/commit.o build/sha1.o build/log.o build/diff.o build/status.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

build/main.o: src/main.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

build/repository.o: src/repository.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/repository.c -o build/repository.o

build/utils.o: src/utils.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/utils.c -o build/utils.o

build/object.o: src/object.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/object.c -o build/object.o

build/index.o: src/index.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/index.c -o build/index.o

build/commit.o: src/commit.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/commit.c -o build/commit.o

build/sha1.o: src/sha1.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/sha1.c -o build/sha1.o

build/log.o: src/log.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/log.c -o build/log.o

build/diff.o: src/diff.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/diff.c -o build/diff.o

build/status.o: src/status.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c src/status.c -o build/status.o

clean:
	rm -rf build $(TARGET)

.PHONY: all clean