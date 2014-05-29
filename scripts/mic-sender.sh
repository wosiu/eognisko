#!/bin/bash

arecord -v -t raw -f cd -B 100000 -D sysdefault | \
    ../client -p 16071 -s localhost "$@" > /dev/null
