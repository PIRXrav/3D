CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Werror -g -O2 -Iinclude/ -Isrc/
LDFLAGS = -L./lib -I./include -lSDL2-2.0 -lm
EXEC = bin/main
SRC=$(shell find src/ -type f -name '*.c')
OBJ=$(patsubst src/%.c,obj/%.o,$(SRC))
DIRS=$(patsubst src/%,obj/%,$(shell find src/ -type d)) bin/tests obj/tests
TESTS=$(wildcard tests/*.c)
TEST_BINS=$(patsubst tests/%.c,bin/tests/%,$(TESTS))


all: $(DIRS) $(EXEC)


$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


tests: $(DIRS) $(TEST_BINS)
	@for t in $(TEST_BINS) ; do \
		if ! $$t ; then \
			echo "Failure at test $$t" ; \
		fi ; \
	done

bin/tests/%: obj/tests/%.o $(filter-out obj/main.o,$(OBJ))
	$(CC) -o $@ $^ $(LDFLAGS)

obj/tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


$(DIRS): %:
	mkdir -p $@

clean:
	rm -rf obj/
	rm -rf bin/

.PHONY: all tests clean
