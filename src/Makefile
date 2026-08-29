BSWORD_ROGUE=BloodSwordRogue.cpp
BSWORD_ROGUE_OUTPUT=BloodSwordRogue.exe
MAP_MAKER=MapMaker.cpp
MAP_MAKER_OUTPUT=MapMaker.exe
DEBUG=-g -O0
LIBS=-O3 -std=c++17 -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lminizip
INCLUDES=-I/usr/include/SDL2 -I/usr/include/minizip
UNAME_S=$(shell uname -s)
UNAME_M=$(shell uname -m)
DEBIAN_ARCH=amd64
DEBIAN=blood-sword.$(DEBIAN_ARCH)

ifeq ($(UNAME_S),Darwin)
	DEBIAN_ARCH=
	DEBIAN=
	LLVM=$(shell brew --prefix llvm)
	CC=$(LLVM)/bin/clang++
	LIBS += -stdlib=libc++ -L$(LLVM)/lib/c++ -L/usr/local/lib
	INCLUDES += -I/usr/local/include/SDL2 -I/usr/local/include/minizip
else ifeq ($(UNAME_M),aarch64)
	DEBIAN_ARCH=arm64
	DEBIAN=blood-sword.$(DEBIAN_ARCH)
	CC=g++
	LIBS += -lstdc++fs
else ifeq ($(UNAME_M),aarch32)
	DEBIAN_ARCH=
	DEBIAN=
	LIBS += -lstdc++fs
	CC=g++
else
	CC=clang++
	LIBS += -stdlib=libc++
endif

.PHONY: all clean

all: clean rogue map-maker

rogue:
	$(CC) -Wall $(BSWORD_ROGUE) -DDEBUG $(INCLUDES) $(LIBS) -o $(BSWORD_ROGUE_OUTPUT)

map-maker:
	$(CC) -Wall $(MAP_MAKER) -DDEBUG $(INCLUDES) $(LIBS) -o $(MAP_MAKER_OUTPUT)

clean-rogue:
	rm -f $(BSWORD_ROGUE_OUTPUT) *.o

clean-map-maker:
	rm -f $(MAP_MAKER_OUTPUT) *.o

clean: clean-rogue clean-map-maker

.ONESHELL: