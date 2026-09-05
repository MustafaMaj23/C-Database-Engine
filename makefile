CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

all: db_engine

db_engine: main.o pager.o btree.o
	$(CC) $(CFLAGS) -o db_engine main.o pager.o btree.o

main.o: main.c db.h
	$(CC) $(CFLAGS) -c main.c

pager.o: pager.c db.h
	$(CC) $(CFLAGS) -c pager.c

btree.o: btree.c db.h
	$(CC) $(CFLAGS) -c btree.c

clean:
	rm -f *.o db_engine *.db