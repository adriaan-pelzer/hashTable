CFLAGS=-g -Wall -fPIC -I/usr/local/include
LIBS=-L/usr/local/lib

all: libHashTable

test: testHashTable
	./testHashTable test-names.txt

testHashTable: Makefile hashTable.h testHashTable.c
	gcc ${CFLAGS} -o testHashTable testHashTable.c ${LIBS} -lHashTable

libHashTable: Makefile hashTable.o hashTable.h
	gcc -shared -o libHashTable.so.1.0 hashTable.o ${LIBS}

hashTable.o: Makefile hashTable.h hashTable.c
	gcc ${CFLAGS} -c hashTable.c -o hashTable.o

install:
	cp libHashTable.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libHashTable.so.1.0 /usr/local/lib/libHashTable.so.1
	ln -sf /usr/local/lib/libHashTable.so.1.0 /usr/local/lib/libHashTable.so
	ldconfig /usr/local/lib
	cp hashTable.h /usr/local/include/hashTable.h

clean:
	rm *.o; rm *.so*; rm core*; rm testHashTable
