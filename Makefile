# The name of the source files
SOURCES = src/server.c lib/net_utils.c

# The name of the executable
EXE = build/server

# Flags for compilation (adding warnings are always good)
CFLAGS = -Wall -pedantic

# Flags for linking (none for the moment)
LDFLAGS =

# Use the GCC frontend program when linking
LD = gcc

# This creates a list of object files from the source files
OBJECTS = $(SOURCES:%.c=%.o)

# The first target, this will be the default target if none is specified
default: all

# Having an "all" target is customary, so one could write "make all"
all: $(EXE)

$(EXE): $(OBJECTS)
	mkdir -p build
	$(LD) $(CFLAGS) $(OBJECTS) -o $(EXE)

# This target compiles all needed source files into object files
# Adding a command to compile the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXE)
	@echo "Running server on port: $(PORT)"
	./$(EXE) -p $(PORT)

# Target to clean up after us
clean:
	-rm -f $(EXE)      # Remove the executable file
	-rm -f $(OBJECTS)  # Remove the object files
	-rmdir build
