#!/bin/bash
../configure \
--with-yices2=no --with-cvc4=no \
CFLAGS="-Wall -O3 -fno-stack-protector -fstrict-aliasing" \
CXXFLAGS="-Wall -O3 -fno-stack-protector -fstrict-aliasing" \
CPPFLAGS="-I/usr/local/include" \
LDFLAGS="-L/usr/local/lib" \
BUDDY_LIB="/usr/local/lib/libbdd.a" \
TECLA_LIBS="/usr/local/lib/libtecla.a" \
GMP_LIBS="/usr/local/lib/libgmpxx.a /usr/local/lib/libgmp.a" \
LIBS="-l:libncursesw.a -l:libtinfo.a"
