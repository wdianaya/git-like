all: app

clean:
	rm -rf app main.o exist_dir.o

app: main.o exist_dir.o           
	gcc -o app main.o exist_dir.o -lssl -lcrypto 
# -lssl -lcrypto флаги для подключения openssl/sha.h

main.o: main.c                   
	gcc -c main.c -o main.o

exist_dir.o: exist_dir.c
	gcc -c exist_dir.c -o exist_dir.o 