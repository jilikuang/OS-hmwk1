CC := gcc
CFLAGS := -Wall -Werror
LDFLAGS :=


OBJECTS := shell.o

OBJECTS_DEBUG := shell_debug.o

all: w4118_sh


w4118_sh: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

%_debug.o: %.c
	$(CC) $(CLFAGS) -DSHELL_DEBUG -o $@ -c $^

debug: $(OBJECTS_DEBUG)
	$(CC) $(CLFAGS) -DSHELL_DEBUG -o w4118_sh_debug $(OBJECTS_DEBUG)

clean:
	rm -f w4118_sh
	rm -f shell.o
	rm -f w4118_sh_debug
	rm -f $(OBJECTS_DEBUG)

.PHONY: clean debug
