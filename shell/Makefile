CC := gcc
CFLAGS := -Wall -Werror
LDFLAGS :=


OBJECTS := shell.o path.o input.o

OBJECTS_DEBUG := shell_debug.o path_debug.o input_debug.o

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
	rm -f w4118_sh_debug
	rm -f $(OBJECTS)
	rm -f $(OBJECTS_DEBUG)
	rm -f err.log

.PHONY: clean debug
