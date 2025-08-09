#!/bin/sh
set -e

rm -rf build
mkdir -p build
gmake clean
gmake CC=owcc\ -bpmodew EXEC=.exe
mv lha.exe build/lhados.exe

gmake clean
gmake CC=i686-w64-mingw32-gcc EXEC=.exe
mv lha.exe build/lhaw32.exe

gmake clean
gmake CC=x86_64-w64-mingw32-gcc EXEC=.exe
mv lha.exe build/lhaw64.exe
