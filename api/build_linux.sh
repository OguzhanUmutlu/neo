gcc -c neo.c -Iinclude
gcc -c types/*.c -Iinclude

mkdir -p build
ar rcs build/libneo-linux.a ./*.o
rm ./*.o
echo Built libneo-linux.a