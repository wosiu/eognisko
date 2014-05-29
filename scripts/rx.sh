#!/bin/bash

../client -s localhost -p 16071 "$@" | \
   aplay -t raw -f cd -B 5000 -v - -D sysdefault
