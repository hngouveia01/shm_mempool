CC=gcc
LIBS+= -lpthread -lrt

main: shm_mempool.o
	$(CC) $(LIBS) shm_mempool.o main.c -o main

shm_mempool.o:
	$(CC) $(LIBS) -c shm_mempool.c -o shm_mempool.o

clean:
	rm -rf *.o main

