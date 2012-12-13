CC=gcc
CFLAGS=-g -Wall -O2 -D_GNU_SOURCE -std=gnu99 `pkg-config --cflags sdl2` `pkg-config --cflags SDL2_ttf`
LDFLAGS=
LIBS=`pkg-config --libs sdl2` `pkg-config --libs SDL2_ttf`
SRC=sdltest.c
OBJ=$(SRC:.c=.o)
OUT=sdltest

all: $(OBJ) $(OUT)

default: all

.SUFFIXES: .c .o

.PHONY: all clean

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJ) core $(OUT).core $(OUT)
