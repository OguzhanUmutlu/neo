clang -c neo.c -Iinclude
clang -c types/*.c -Iinclude

mkdir -p build
libtool -static -o build/libneo-mac.a neo.o types/*.o
rm neo.o types/*.o
echo Built libneo-mac.a