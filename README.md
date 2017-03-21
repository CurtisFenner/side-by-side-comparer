# side-by-side-comparer
This is a small C program to compare text files side-by-side.

## How to build it

`build.bat` is a short batch/bash script to build the program.

It has no dependencies except C99 and POSIX extensions; simply `gcc comparer.c` will build the executable.

`comparer.c` and `vector.h` are the only source files needed to build `comparer`.

This should work with no extra incantations on Mac, Linux, Windows Linux Subsystem, and MinGW/Cygwin.

## How to use it

Give the `comparer` executable two filenames as command-line arguments.

![> comparer bef.txt aft.txt / red (on left) & green (on right) colored output comparing two sample files](https://github.com/CurtisFenner/side-by-side-comparer/blob/master/exampleoutput.png?raw=true)

The output is colored with ANSI colors and should work on most terminals, but probably shouldn't be saved to a file.

## How it works

Like the standard `diff` tool, it finds a longest-common-subsequence between the two input files' lines, and
considers any lines not part of that common subsequence to be deletions/insertions.
