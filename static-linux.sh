#!/bin/bash
../configure \
--with-cvc4=no \
CFLAGS="-Wall -O2 -static-libstdc++ -static-libgcc -fno-crossjumping -fno-stack-protector -fstrict-aliasing -finline-limit=10000" \
CXXFLAGS="-Wall -O2 -static-libstdc++ -static-libgcc -fno-crossjumping -fno-stack-protector -fstrict-aliasing -finline-limit=10000" \
CPPFLAGS="-I/usr/local/include" \
LDFLAGS="-static -L/usr/local/lib" \
BUDDY_LIB="/usr/local/lib/libbdd.a" \
TECLA_LIBS="/usr/local/lib/libtecla.a /usr/lib/libncurses.a" \
GMP_LIBS="/usr/local/lib/libgmpxx.a /usr/local/lib/libgmp.a" \
LIBSIGSEGV_LIB="/usr/local/lib/libsigsegv.a"
