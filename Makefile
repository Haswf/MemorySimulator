## Source files
src = $(wildcard src/*.c)

DEPS = $(wildcard src/*.h)

OBJ = $(src:.c=.o)

EXE = scheduler
CFLAGS = -g -Wall -O3 -std=gnu99
CC = gcc

## Create .o files from .c files. Searches for .c files with same .o names given in OBJ
$(EXE): $(OBJ)
	$(CC) -o $@ $^ -lm $(CFLAGS)

## Create executable linked file from object files.
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ -lm $<

# Perform clean
clean:
	find . -name '*.o' -delete
	rm -f $(EXE)