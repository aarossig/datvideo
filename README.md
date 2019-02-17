# DAT Video

## Overview

This project contains utilities for reading and writing arbitrary data to DAT
(digital audio tape) magnetic tapes. The DAT format specifies 48kHz, 2-channel,
16-bit PCM audio which lends itself to a rather high data rate of ~188kB/s.
This is suitable for compressed video!

This tool is really just a minimalistic RFC-1662 framer that does encode and
decode. It could be used for probably more things than storing binary data on
a DAT tape - but that is why it exists in the first place.

## Building

Build this tree with the usual cmake flow:

    mkdir build
    cd build
    cmake ..
    make -j`nproc`

## Usage

You can run ./datvideo --help to receive a usage listing. Here are some examples
of how it might be used with some ALSA utilities:

### Encoding

    ./src/datvideo --encode \
        -i ~/Downloads/bbb_sunflower_1080p_60fps_normal_one_audio_h265.ts \
        --chunk_size 188 -o big_buck_bunny.bin
    aplay --format=S16_LE --device=hw:2,0 \
        --channels=2 --rate=48000 --file-type=raw big_buck_bunny.bin

### Decoding

    arecord --format=S16_LE \
        --device=hw:2,0 --channels=2 --rate=48000 \
        --file-type=raw | ./src/datvideo --decode -o bbb.ts
