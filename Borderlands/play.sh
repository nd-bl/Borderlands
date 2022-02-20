#!/bin/sh

# play, from sox
#play $@ -r 44100 -b 16 -c 1 -e float

# alsa play
aplay -q -c 2 -t raw -f FLOAT_LE -r 44100 $@
