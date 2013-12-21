@echo off

mcfbuild -C"mcfcrt" %*

g++ -std=c++11 -Wall -Wextra -fno-builtin main.cpp -Lmcfcrt\.built-default\ -static -nostartfiles -Wl,-e__MCFExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,-lmcfcrt,-lstdc++,-lgcc,-lgcc_eh,-lmingwex,-lmcfcrt
