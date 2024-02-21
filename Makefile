CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11 -O0 -ggdb -no-pie
LIBS:=-lm

all: cachesim

cachesim: cachesim.o lib_c.o cache_c.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

cachesim.o: cachesim.c 
	$(CC) $(CFLAGS) -c $^ -o $@

lib_c.o: lib.c lib.h
	$(CC) $(CFLAGS) -c $< -o $@

cache_c.o: cache.c cache.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f *.o
	rm -f cachesim
	

