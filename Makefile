TARGET=bin\C8EMU.exe


CC=gcc
RAYLIB_INSTALL_DIR=C:\raylib\raylib\src

SRC = $(wildcard src/*.c)
OBJS=$(patsubst src/%.c, objs/%.o, $(SRC))

CFLAGS= -I$(RAYLIB_INSTALL_DIR)
LDFLAGS= -Llib -lraylib-nolog -lgdi32 -luser32 -lwinmm

all: dirs $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

objs/C8EMU.o: src/C8EMU.c src/C8EMU.h

objs/main.o: src/C8EMU.h

objs/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

dirs:
	@if not exist objs mkdir objs


clean:
	-del $(TARGET) objs\*.o