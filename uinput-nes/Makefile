#gmake Makefile
EXE = uinput-nes
SHELL = bash

CFLAGS = -Wall -ggdb3
LFLAGS = -lftdi
CC     = gcc
OBJS   = $(patsubst %.c, %.o, $(wildcard src/*.c))

.PHONY: all clean

all: post-build

pre-build:
	@src/update-version
	@echo

post-build: main-build
	#	@strip $(EXE)
	@echo
	@echo "Success: $(EXE)"

main-build: pre-build
	@$(MAKE) --no-print-directory $(EXE)

$(EXE): $(OBJS)
	@echo -e "\e[4;1m$@\e[0m"
	$(CC) $(CFLAGS) $(LFLAGS) -o $(EXE) $(OBJS)

%.o: %.c
	@echo -e "\e[4;1m$@\e[0m"
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo

clean:
	-rm -f $(OBJS) $(EXE)
