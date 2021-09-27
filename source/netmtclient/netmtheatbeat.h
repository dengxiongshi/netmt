#ifndef __HEATBEAT_H__
#define __HEATBEAT_H__

#include <signal.h>
#include "Channel.h"


using namespace hv;
// #include "hsocket.h"

// #define SleepTime 1 //s
// #define ConnTime 1000 //ms

// bool IsConn(hio_t* io) {
//     hio_set_connect_timeout(io, ConnTime);
//     hio_set_heartbeat(io, 1000, SocketChannelPtr->send_hearbeat);
//     hv_sleep(SleepTime);
// }

SocketChannelPtr sockcp;
std::string str = sockcp->localaddr();

bool IsState(int sockfd) {
    if (tcp_keepalive(sockfd) != 0) {
        return false;
    }else {
        return true;
    }
}

typedef void (*signal_handler)(int);

void heart_handler_fun(int signal_num) {
    hv_sleep(30); //睡眠30s
    printf("catch signal %d\n", signal_num);
}


#endif 