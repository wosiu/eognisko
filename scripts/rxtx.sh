#!/bin/bash

sox -q "messier.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | \
   ../client -p 16071 -s localhost "$@" | \
#  ../client -s students.mimuw.edu.pl -p 16079 "$@" | \
   aplay -t raw -f cd -B 5000 -v -D sysdefault -
