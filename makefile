PROGRAM=test.cgi
CFLAGS=-g -ggdb3 -O0 -Wall -pedantic -std=c99
OBJECTS_DIR=objects
VERSION=0.1
SRC_DIR=src
OBJECTS=main.o webcz.o strbuf.o

default: all
	chown www:www $(PROGRAM)
	chmod +x $(PROGRAM)

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS_DIR)/*.o -o $(PROGRAM)

main.o:	main.c
	$(CC) -c $(CFLAGS) main.c -o $(OBJECTS_DIR)/$@

webcz.o: $(SRC_DIR)/webcz.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/webcz.c -o $(OBJECTS_DIR)/$@

strbuf.o: $(SRC_DIR)/strbuf.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/strbuf.c -o $(OBJECTS_DIR)/$@

clean:
	-rm $(PROGRAM)
	-rm $(OBJECTS_DIR)/*.o
