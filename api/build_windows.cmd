@echo off

pushd %~dp0

gcc -c neo.c -Iinclude -lgmp -lmpfr -lm
gcc -c types/*.c -Iinclude -lgmp -lmpfr -lm

mkdir build >nul 2>&1
ar rcs build/libneo-windows.a ./*.o
del .\*.o
echo Built libneo-windows.a

popd