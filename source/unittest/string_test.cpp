#include "string_ini.h"

int main() {
    char *str = (char*)"aaa   fe";
    int num = commandNum(str, splitstr);
    if (num == 1) {
        printf("%s\n", str);
    }else {
        char **command = strsplit(str, splitstr);
    
        for (int i = 0; i < num; i++) {
            for (int j = 0; j < strlen(command[i]); j++) {
                printf("%c", command[i][j]);
            }
            printf("\n");
        }
    }
    return 0;
}

// #include "hstring.h"

// int main(int argc, char* argv[]) {
//     std:string str("aaa fe");
//     StringList strlist = split(str, ' ');
//     printf("split %s\n", str.c_str());
//     int i = 0;
//     for (auto &str : strlist)
//     {
//         i++;
//         printf("%s\n", str.c_str());
//     }
//     printf("%d\n", i);
// }