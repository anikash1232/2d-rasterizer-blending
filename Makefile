# define CPPFLAGS=-I... for other (system) includes
# define LDFLAGS=-L... for other (system) libs to link

CC = g++ -g -Wno-narrowing -Wreturn-type -Wunused-function -Wreorder -Wunused-variable -Wfloat-conversion

CC_DEBUG = @$(CC) -std=c++17
CC_RELEASE = @$(CC) -std=c++17 -O3 -DNDEBUG

G_DEPS = $(wildcard *.cpp *.h apps/* src/* include/*)

G_SRC = $(wildcard src/*.cpp *.cpp)

G_INC = $(CPPFLAGS)

G_LINK = $(LDFLAGS)

all: image tests bench dbench

image : $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/image/*.cpp -o image

tests : $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/tests/*.cpp -o tests

bench : $(G_DEPS)
	$(CC_RELEASE) $(G_INC) $(G_SRC) apps/bench/*.cpp -o bench

# debug variant of bench -- not any good for timing, but helps debugging --once
dbench : $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/bench/*.cpp -o dbench

DRAW_SRC  = apps/demos/draw*.cpp apps/demos/game_font.cpp apps/demos/GWindow.cpp
GAME_SRC = apps/demos/game*.cpp apps/demos/GWindow.cpp
ROCKS_SRC = apps/demos/rocks.cpp apps/demos/GWindow.cpp

draw: $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) $(G_LINK) $(DRAW_SRC) -lSDL2 -o draw

dgame: $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) $(G_LINK) $(GAME_SRC) -lSDL2 -o dgame

game: $(G_DEPS)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(G_LINK) $(GAME_SRC) -lSDL2 -o game

rocks: $(G_DEPS)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(G_LINK) $(ROCKS_SRC) -lSDL2 -o rocks
drocks: $(G_DEPS)
	$(CC_DEBUG) $(G_INC) $(G_SRC) $(G_LINK) $(ROCKS_SRC) -lSDL2 -o drocks

clean:
	@rm -rf image tests bench dbench draw dgame game rocks drocks pa?_*.png *.dSYM *.exe

