PROGRAM=index.cgi
CFLAGS=-g -ggdb3 -O0 -Wall -pedantic -std=c99
OBJECTS_DIR=objects
VERSION=0.1
SRC_DIR=src
PKGS=openssl
FLAGS = $(shell pkg-config --libs --cflags $(PKGS))
INCLUDES = $(shell pkg-config --cflags $(PKGS))
OBJECTS=objects/main.o objects/webcz.o objects/strbuf.o

default: SETUP $(PROGRAM)
	chmod +x $(PROGRAM)

SETUP:
	@echo "Building $(PROGRAM)";
	if [ ! -d $(OBJECTS_DIR) ]; then \
		mkdir $(OBJECTS_DIR); \
	fi
	if [ ! -d sessions ]; then \
		mkdir sessions; \
	fi

$(PROGRAM): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $@

objects/main.o:	main.c
	$(CC) -c $(CFLAGS) main.c -o $@

objects/webcz.o: $(SRC_DIR)/webcz.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(SRC_DIR)/webcz.c -o $@

objects/strbuf.o: $(SRC_DIR)/strbuf.c
	$(CC) -c $(CFLAGS) $(SRC_DIR)/strbuf.c -o $@

clean:
	-rm $(PROGRAM)
	-rm $(OBJECTS_DIR)/*.o
