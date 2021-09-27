#include "touch.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("touch fatal!");
    }
    char *filename = argv[1];
    mytouch(filename);
}