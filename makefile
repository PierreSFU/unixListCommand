# Makefile for building demo applications.
# by Brian Fraser, edited by Pierre Drego :)

# Edit this file to compile extra C files into their own programs.
TARGETS= myls

CROSS_TOOL = 
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = getFiles.c -o assignment4 -Wall -g
all: clean $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $@.c -o $@ 

clean:
	rm -f $(TARGETS)