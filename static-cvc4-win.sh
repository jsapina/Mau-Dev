#!/bin/bash
../configure \
--with-yices2=no \
CFLAGS="-Wall -O3 -static-libstdc++ -static-libgcc -fno-stack-protector -fstrict-aliasing" \
CXXFLAGS="-Wall -O3 -static-libstdc++ -static-libgcc -fno-crossjumping -fno-stack-protector" \
CPPFLAGS="-I/usr/local/include" \
LDFLAGS="-static -L/usr/local/lib -Wl,--stack,16777216" \
BUDDY_LIB="/usr/local/lib/libbdd.a" \
TECLA_LIBS="/usr/local/lib/libtecla.a /usr/lib/libncurses.a" \
GMP_LIBS="/usr/local/lib/libgmpxx.a /usr/local/lib/libgmp.a" \
LIBSIGSEGV_LIB="/lib/libsigsegv.dll.a"
