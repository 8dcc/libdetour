
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra

SRC=main.c libdetour.c
OBJ=$(addprefix obj/, $(addsuffix .o, $(SRC)))

BIN=libdetour-test.out

#-------------------------------------------------------------------------------

.PHONY: all all-32bit flags-32bit clean

all: $(BIN)
all-32bit: flags-32bit all

flags-32bit:
	$(eval CFLAGS += -m32)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

#-------------------------------------------------------------------------------

%.out: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<
