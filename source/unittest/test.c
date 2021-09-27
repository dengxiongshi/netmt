#include <string.h>
#include <stdio.h>
#include "command.h"


int main() {
    char* strback = "PONG\r\n";
    printf("%d\n", strlen(strback));
}

int show_netmt_version(struct cmd_element* , struct vty* , int, char **);
struct cmd_element show_netmt_version_cmd = {"show netmt version", show_netmt_version, "Show running system information\n"
                                                                                       "show netmt version\n"};
int show_netmt_version(struct cmd_element *self, struct vty *vty, int argc, char **argv) {
    show_version(vty);
    return CMD_SUCCESS;
}
