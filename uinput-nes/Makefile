#gmake Makefile
EXE = uinput-nes
SHELL = bash

CFLAGS = -Wall -ggdb3
LFLAGS = -lftdi1 # '-lftdi' on older systems
CC     = gcc
OBJS   = $(patsubst %.c, %.o, $(wildcard src/*.c))

.PHONY: all clean

all: post-build

pre-build:
	@src/update-version
	@echo

post-build: main-build
	@strip $(EXE)
	@echo
	@echo "Success: $(EXE)"

main-build: pre-build
	@$(MAKE) --no-print-directory $(EXE)

$(EXE): $(OBJS)
	@echo -e "\e[4;1m$@\e[0m"
	$(CC) -o $(EXE) $(OBJS) $(LFLAGS)

%.o: %.c
	@echo -e "\e[4;1m$@\e[0m"
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo

clean:
	-rm -f $(OBJS) $(EXE)
