CFLAGS ?= -O2
LDFLAGS = $(shell pkg-config --libs --cflags raylib) -lm

all: game

game: game.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f game
