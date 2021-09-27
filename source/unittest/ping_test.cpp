// #ifdef __cplusplus
// extern "C" {
// #endif

#include "ping.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ping host|ip\n");
        return -10;
    }

    char *host = argv[1];
    int ping_cnt = 4;
    bool ok_cnt = _Ping(argc, argv, host, ping_cnt);
    if (ok_cnt) {
        printf("Ping success!\n");
    } else {
        printf("Ping fatal!\n");
    }
    return 0;
}

// #ifdef __cplusplus
// };
// #endif