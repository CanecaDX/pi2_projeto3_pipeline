
APP := mars_c

CC ?= gcc

CFLAGS ?= -Wall -Wextra -g -O2 -Iinclude

SRCS := mars_main.c \
	src/controle.c \
	src/decoder.c \
	src/extensor.c \
	src/memory.c \
	src/multiciclo.c \
	src/pc.c \
	src/registers.c \
	src/ULA.c

OBJS := $(SRCS:.c=.o)

.PHONY: all run clean 

all: $(APP)

$(APP): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(APP)
	./$(APP)

clean:
	rm -f $(OBJS) $(APP)

help:
	@echo "Usage:"
	@echo "  make        - Build"
	@echo "  make run    - Run"
	@echo "  make clean  - Clean up build files"
