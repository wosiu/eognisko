#!/bin/bash

sox -q $1 -r 44100 -b 16 -e signed-integer -c 2 -t raw - | \
   ../client -p 16071 -s localhost
