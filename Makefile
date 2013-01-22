CC=gcc
CFLAGS=-g -Wall -O2 -D_GNU_SOURCE -std=gnu99 `pkg-config --cflags sdl2` `pkg-config --cflags SDL2_ttf`
LDFLAGS=
LIBS=`pkg-config --libs sdl2` `pkg-config --libs SDL2_ttf`
SRC=sdltest.c utc_time.c
OBJ=$(SRC:.c=.o)
OUT=sdltest
ARCH := $(shell uname -s)

ifeq ($(ARCH),Darwin)
else
ifeq ($(ARCH),Linux)
	LIBS += -lrt
endif
endif


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
