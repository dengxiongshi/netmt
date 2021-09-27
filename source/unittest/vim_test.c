#include "vim.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        //没有命令行的参数
        return -1;
    }
    char *file = argv[1];
    myvim(file);
    return 0;
}