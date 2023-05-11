CC=g++
CFLAGS=-I

client: client.cpp Utils/helpers.cpp Utils/Buffer/buffer.cpp
	$(CC) -o client client.cpp Utils/helpers.cpp Utils/Buffer/buffer.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
