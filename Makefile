
CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=

OBJ_FILES=main.c.o libdetour.c.o
OBJS=$(addprefix obj/, $(OBJ_FILES))

BIN=detour-test.out

.PHONY: all clean flags-32bit all-32bit

# -------------------------------------------

all: $(BIN)

clean:
	rm -f $(OBJS)
	rm -f $(BIN)

# Add -m32 for x86 systems stuff
flags-32bit:
	$(eval CFLAGS += -m32)

all-32bit: flags-32bit all

# -------------------------------------------

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

