
CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=

OBJS=obj/main.c.o obj/detour.c.o
BIN=detour-test.out

.PHONY: clean all flags-32bit all-32bit

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

$(OBJS): obj/%.c.o : src/%.c
	@mkdir -p obj/
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

