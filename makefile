CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lwiringPi -lncurses
SOURCES=gbapio.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=gbapio

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
