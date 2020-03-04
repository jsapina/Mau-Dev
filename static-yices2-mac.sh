#!/bin/bash
../configure \
--with-cvc4=no \
CFLAGS="-Wall -O3 -static-libstdc++ -static-libgcc -fno-stack-protector -fstrict-aliasing" \
CXXFLAGS="-Wall -O3 -static-libstdc++ -static-libgcc -fno-stack-protector -fstrict-aliasing" \
BUDDY_LIB="/usr/local/lib/libbdd.a" \
TECLA_LIBS="/usr/local/lib/libtecla.a /opt/local/lib/libncurses.a" \
GMP_LIBS="/usr/local/lib/libgmpxx.a /usr/local/lib/libgmp.a" \
LIBSIGSEGV_LIB="/usr/local/lib/libsigsegv.a" \
YICES2_LIB="/usr/local/lib/libyices.dll.a"
