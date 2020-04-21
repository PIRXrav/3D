CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -g -O2 -Iinclude/ -fPIC
LDFLAGS = -L./lib -I./include -lSDL2-2.0 -lm
EXEC = main
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,obj/%.o,$(SRC))

# .PHONY: tests

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


tests:
	echo $(SRC)
	echo $(OBJ)

.PHONY: clean

clean:
	rm -rf obj/*.o
	rm $(EXEC)
