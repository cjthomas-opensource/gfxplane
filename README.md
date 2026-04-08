# Simple Graphics Surface Library

## Overview

This project implements a C++ library that provides a 2D graphics surface
with a few drawing primitives and with image file I/O functions.

The idea is to be able to quickly create, manipulate, read, and write images
without having to perform complex setup or having to sort out complex
dependencies.

This is intended for simple toy-program applications (things like "draw a
mandelbrot set", not "make a video game").

![Mandelbrot test image.](./assets/mandel.png)

## Folders

`library` - The library itself. Makefile targets:
  `clean` - Remove the local copy of the library binary.
  `debug` - Compile a version of the binary with no optimization.
  `default` - Compile the optimized version of the binary.
  `install` - Copy the binary to the `$(APPROOT)/lib` folder and the header to `$(APPROOT)/include`.

`test` - Test programs. Makefile targets:
  `all` - Build all of the test programs.
  `clean` - Remove the test program binaries.
  `outclean` - Remove the output images.
  `run` - Run the test program binaries, generating output images.

_(This is the end of the file.)_
