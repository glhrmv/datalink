# Program name
PROG := datalink

# Compiler flags
CFLAGS := -Wall -Wextra

# GCC flags
CCFLAGS = -g -O3 -Wall

# Source files
SRCS := $(wildcard ./src/*.c)

# Object and dependencies (file names)
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

# Output binary folder
OUT = bin

# Called when you run 'make'. This calls the line below.
all: $(PROG)

$(PROG): $(OBJS)
	mkdir -p $(OUT)
	gcc $(CFLAGS) $(OBJS) -o $(OUT)/$(PROG)

# Includes the dependency lists (.d files).
-include $(DEPS)

clean:
	rm -f $(OUT)/$(PROG)
	rm -f $(DEPS) $(OBJS)

