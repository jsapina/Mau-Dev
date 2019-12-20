#!/bin/bash
../configure \
CFLAGS="-Wall -O3 -static-libstdc++ -static-libgcc -fno-crossjumping -fno-stack-protector -fstrict-aliasing -mmacosx-version-min=10.5 -finline-limit=10000" \
CXXFLAGS="-Wall -O3 -static-libstdc++ -static-libgcc -fno-crossjumping -fno-stack-protector -fstrict-aliasing -mmacosx-version-min=10.5 -finline-limit=10000" \
BUDDY_LIB="/usr/local/lib/libbdd.a" \
TECLA_LIBS="/usr/local/lib/libtecla.a /opt/local/lib/libncurses.a" \
GMP_LIBS="/usr/local/lib/libgmpxx.a /usr/local/lib/libgmp.a" \
LIBSIGSEGV_LIB="/usr/local/lib/libsigsegv.a"
