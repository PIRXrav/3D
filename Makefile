CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra  -g -Iinclude/ -Isrc/ -fno-stack-protector
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

assets:
	mkdir -p data/extern/
	wget https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/pumpkin_tall_10k.obj -O data/extern/pumpkin.obj;
	wget https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/teapot.obj -O data/extern/teapot.obj;
	wget https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/cow-nonormals.obj -O data/extern/cow.obj;
	wget https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/teddy.obj -O data/extern/teddy.obj;
	wget https://graphics.stanford.edu/~mdfisher/Data/Meshes/bunny.obj -O data/extern/bunny.obj

asset_elephant:
	wget http://graphics.im.ntu.edu.tw/~robin/courses/gm05/model/elephav.obj.gz -O /tmp/xxx.obj.gz;
	gunzip /tmp/xxx.obj.gz
	mv /tmp/xxx.obj  data/extern/elephant.obj


asset_mba:
	wget http://graphics.im.ntu.edu.tw/~robin/courses/gm05/model/mba1.obj.gz -O /tmp/xxx.obj.gz;
	gunzip /tmp/xxx.obj.gz
	mv /tmp/xxx.obj  data/extern/mba.obj

asset_venus:
	wget http://graphics.im.ntu.edu.tw/~robin/courses/gm05/model/venusv.obj.gz -O /tmp/xxx.obj.gz;
	gunzip /tmp/xxx.obj.gz
	mv /tmp/xxx.obj  data/extern/venus.obj

clean:
	rm -rf obj/
	rm -rf bin/

.PHONY: all tests clean
