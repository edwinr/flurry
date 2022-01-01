#!/bin/sh

set -e

mips64r5900el-ps2-elf-g++ \
    -D_EE \
    -G0 \
    -O2 \
    -Wall \
    -I/usr/local/ps2dev/ps2sdk/ee/include \
    -I/usr/local/ps2dev/ps2sdk/common/include \
    -I. \
    -T/usr/local/ps2dev/ps2sdk/ee/startup/linkfile \
    -o flurry.elf \
    *.cpp ../core/*.cpp \
    -L/usr/local/ps2dev/ps2sdk/ee/lib \
    -Wl,-zmax-page-size=128 \
    -ldraw \
    -lgraph \
    -lmath3d \
    -lpacket \
    -ldma
cp flurry.elf flurry_unstripped.elf
mips64r5900el-ps2-elf-strip --strip-all flurry.elf