export ROOT_DIR=$(shell pwd)
export SRC_DIR	= src
export INCL_DIR = include
export EXPL_DIR = examples
export TEST_DIR = test

export CC=gcc

# Debugage
export CFLAGS=-Wall -g -DDEBUG_NDES
export LDFLAGS=-g 

# Performances
export CFLAGS=-Wall -g -DNDEBUG -O3
export LDFLAGS=-g -O3

default : src 

all : src tests examples 

.PHONY: clean src examples test

src : 
	@(cd $(SRC_DIR) && $(MAKE))

examples : 
	@(cd $(EXPL_DIR) && $(MAKE))

install : knapsack
	strip $(KNAPSACK_DIR)/knapsack
	rm $(HOME)/bin/knapsack
	cp $(KNAPSACK_DIR)/knapsack $(HOME)/bin

tests : 
	@(cd $(TEST_DIR) && $(MAKE))

clean :
	@(cd $(SRC_DIR) && $(MAKE) $@)
	@(cd $(EXPL_DIR) && $(MAKE) $@)
	@(cd $(TEST_DIR) && $(MAKE) $@)

.c.o :
	$(CC) -I. $< -c

