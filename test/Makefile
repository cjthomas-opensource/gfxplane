# Simple image rendering and I/O library - Test programs.
# Written by Christopher Thomas.

#
# Flags

GPP=g++
CFLAGS=-g -Wall -I$(APPROOT)/include
LDFLAGS=-L$(APPROOT)/lib -limgcjt

# NOTE - Order of arguments matters.
# Linking flags must be after source files.
# Order of linking flags also matters.


all: mandel conlife grad

force: clean all

clean:
	rm -f mandel
	rm -f conlife
	rm -f grad

outclean:
	rm -f output/*

run: all
	./mandel
	./conlife
	./grad


# Mandelbrot set generator.
# Tests pixel writes and file writes.
mandel: mandel.cpp
	$(GPP) $(CFLAGS) mandel.cpp $(LDFLAGS) -o mandel

# Conway's life simulation.
# Tests pixel reads and a subset of file writes.
conlife: conlife.cpp
	$(GPP) $(CFLAGS) conlife.cpp $(LDFLAGS) -o conlife

# Gradient test for maxval rescaling.
grad: grad.cpp
	$(GPP) $(CFLAGS) grad.cpp $(LDFLAGS) -o grad


#
# This is the end of the file.
