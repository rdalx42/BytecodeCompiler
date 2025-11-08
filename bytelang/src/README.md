# How to compile

Make sure you have C++ 20+ installed 

```bash
cd src # cd into main directory

g++ runtime/*.cpp compiler/*.cpp backend/*.cpp -O3 -ffast-math -march=native -Iinclude -o byt.exe # build in byt.exe

```
