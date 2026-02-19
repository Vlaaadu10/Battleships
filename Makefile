# Copyright PCLP Team, 2025

# Compiler setup.
CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99

# Define targets, e.g., ninel, codeinvim, vectsecv, nomogram.
TARGETS=battleships
OBJ_FILES=$(TARGETS:%=%.o)

# Manually define all targets.
build: $(TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
battleships: battleships.o
	$(CC) $(CFLAGS) $^ -o $@ -lm
# Pack the solution into a zip file.
pack:
	zip -FSr 314CA_AlexandruVladutStefan_Tema2.zip README Makefile *.c *.h

# Clean the solution.
clean:
	rm -f $(TARGETS) $(OBJ_FILES) *.out *.error

.PHONY: build pack clean
