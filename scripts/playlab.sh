#!/bin/bash

lame --decode "messier.mp3" - | \
sox -q - -r 44100 -b 16 -e signed-integer -c 2 -t raw - | \
   ../client -p 16071 -s 10.1.1.123 "$@" | \
#  ../client -s students.mimuw.edu.pl -p 16079 "$@" | \
   aplay -t raw -f cd -B 5000 -v -D sysdefault -
