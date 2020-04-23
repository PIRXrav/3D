CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -g -O2 -Iinclude/ -Isrc/
LDFLAGS = -L./lib -I./include -lSDL2-2.0 -lm
EXEC = main
SRC=$(shell find src/ -type f -name '*.c')
OBJ=$(patsubst src/%.c,obj/%.o,$(SRC))
DIRS=obj obj/containers obj/parsers

# .PHONY: tests

all: $(DIRS) $(EXEC)

$(DIRS): %:
	mkdir -p $@


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
