# Simple image rendering and I/O library.
# Written by Christopher Thomas.
# This is a quick and dirty library for 2D graphics and PPM/CSV file I/O.

#
# Flags

GPP=g++
CFLAGS=-g -Wall -O3
CFLAGSDEBUG=-g -Wall -O0
LDFLAGS=-r -nostdlib

INSTALLDIR=$(APPROOT)


#
# Files

BINARY=libimgcjt.a

SOURCES=\
	imgcjt-image.cpp	\
	imgcjt-io.cpp

HEADERS=\
	imgcjt.h


#
# Targets

default: $(BINARY)

# NOTE - Link flags must be after source files.
$(BINARY): $(SOURCES) $(HEADERS)
	$(GPP) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(BINARY)

debug: $(SOURCES) $(HEADERS)
	$(GPP) $(CFLAGSDEBUG) $(SOURCES) $(LDFLAGS) -o $(BINARY)

install: $(BINARY) $(HEADERS)
	cp $(BINARY) $(INSTALLDIR)/lib
	cp $(HEADERS) $(INSTALLDIR)/include

force: clean default

clean:
	rm -f $(BINARY)

#
# This is the end of the file.
