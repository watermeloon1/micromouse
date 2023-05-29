CC := clang
SRCS := mazer.c
OBJS := $(SRCS:.c=.o)
EXEC := mazer

CFLAGS := $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs)

.PHONY: all clean rebuild

rebuild: clean all

all: $(EXEC)

$(EXEC): $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(OBJS): $(@:.o=.c) Makefile
	$(CC) $(CFLAGS) -o $@ $(@:.o=.c) -c

clean:
	rm -f $(OBJS) $(EXEC)
