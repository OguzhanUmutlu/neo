#include "neo.h"

int main(int argc, char *argv[]) {
    NEO_init(argc, argv);
    internal_NEO_print(NEO_string3("hello, world!"));
    NEO_exit(0);
}