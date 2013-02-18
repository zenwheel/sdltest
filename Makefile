SDL_VERSION=1
CC=gcc
ifeq ($(SDL_VERSION),1)
CFLAGS=-g -Wall -O2 -D_GNU_SOURCE -std=gnu99 `pkg-config --cflags sdl` `pkg-config --cflags SDL_ttf`
LIBS=`pkg-config --libs sdl` `pkg-config --libs SDL_ttf`
else
CFLAGS=-g -Wall -O2 -D_GNU_SOURCE -std=gnu99 `pkg-config --cflags sdl2` `pkg-config --cflags SDL2_ttf`
LIBS=`pkg-config --libs sdl2` `pkg-config --libs SDL2_ttf`
endif
LDFLAGS=
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
