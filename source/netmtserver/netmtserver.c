/*
 * tcp echo server
 *
 * @build   make examples
 * @server  bin/tcp_echo_server 1234
 * @client  bin/nc 127.0.0.1 1234
 *          nc     127.0.0.1 1234
 *          telnet 127.0.0.1 1234
 */

// #include <pthread.h>
#include "hv.h"
#include "event/hevent.h"
#include "netmt.h"
#include "inifile.h"
#include "netmtcommand.h"

server_t t_server;
io_t t_io;
// netmt_command_t *comm;
extern addr_t client_addr;
FILE *fp;
struct stat statbuf;
hio_t *udpio = NULL;

char databuf[DATABUF];
int bytelen = 0;

int udpFD;
static hmutex_t s_mutex;
static int fsize;

//获取客户端的ip和port
static void get_sockaddr(sockaddr_u* addr, io_t* io, int len) {
    char ip[SOCKADDR_STRLEN] = {0};
    uint16_t port = 0;
    if (addr->sa.sa_family == AF_INET) {
        inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
        port = htons(addr->sin.sin_port);
        // io->netmtaddr.netmthost = ip;
        strcpy(io->netmtaddr.netmthost, ip);
        io->netmtaddr.netmtport = port;
    }
    else if (addr->sa.sa_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
        port = htons(addr->sin6.sin6_port);
        // io->netmtaddr.netmthost = ip;
        strcpy(io->netmtaddr.netmthost, ip);
        io->netmtaddr.netmtport = port;
    }
}

inline void conf_ctx_init(conf_ctx_t* ctx) {
    // ctx->parser = new IniParser;
    ctx->loglevel = LOG_LEVEL_DEBUG;
    ctx->worker_processes = 0;
    ctx->worker_threads = 0;
    ctx->port = 0;
}

static void netmt_close(hio_t* io) {
    printf("on_close fd=%d error=%d\n", hio_fd(io), hio_error(io));

    io_t *client = (io_t *)hevent_userdata(io);
    if (client) {
        hevent_set_userdata(io, NULL);

        hmutex_lock(&s_mutex);
        list_del(&client->node);
        hmutex_unlock(&s_mutex);

        HV_FREE(client);
    }
}

//写操作
static void netmt_send(hio_t* io, void* buf, int readbytes) {

    char* str = (char*)buf;
    char eol = str[readbytes-1];
    if (eol == '\n' || eol == '\r') {
        if (readbytes > 1 && str[readbytes-2] == '\r' && eol == '\n') {
            // have been CRLF
        }
        else {
            ++readbytes;
            str[readbytes - 2] = '\r';
            str[readbytes - 1] = '\n';
        }
    }

    if (strncmp(str, "start", 5) == 0) {
        printf("call hio_read_start\n");
        hio_read_start(io);
        return;
    }
    else if (strncmp(str, "stop", 4) == 0) {
        printf("call hio_read_stop\n");
        hio_read_stop(io);
        return;
    }
    else if (strncmp(str, "close", 5) == 0) {
        printf("call hio_close\n");
        hio_close(io);
        return;
    }
    else if (strncmp(str, "quit", 4) == 0) {
        printf("call hloop_stop\n");
        hloop_stop(hevent_loop(io));
        return;
    }
}

void save_result_info_to_buffer(hio_t* io, void* buf, int readbytes)
{
    if (pthread_mutex_lock(&s_mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }

    //按接收到的字符串进行比较
    netmt_command_t* comm = (netmt_command_t*)buf;
    io_t* conn = (io_t*)io->userdata;
    int threshold = conn->buffer_max_size - 4;

    if(conn->buffer_current_size < threshold)
    {
        strcpy(conn->buffer, comm->total_result);
        printf("conn->buffer: \n%s\n", conn->buffer);
    }

    if (pthread_mutex_unlock(&s_mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
     
}

void handle_endflag(hio_t* io, void* buf, int readbytes)
{
    if (pthread_mutex_lock(&s_mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }  

    //按接收到的字符串进行比较
    io_t* conn = (io_t*)io->userdata;

    conn->flag = 1;

    if (pthread_mutex_unlock(&s_mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }    

}

void handle_success(hio_t* io, void* buf, int readbytes)
{
    char* str = (char*)buf;
    io_t* conn = (io_t*)io->userdata;

    if (pthread_mutex_lock(&s_mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }    

    if(strncmp(str, "shell exec success", 15) == 0)
    {
        strcpy(conn->success, str);
    }

    if (pthread_mutex_unlock(&s_mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    } 
}

void handle_ping_message(hio_t* io, void* buf, int readbytes)
{
    //按接收到的字符串进行比较
    char* str = (char*)buf;

    if (strncmp(str, "PING", 4) == 0)
    {
        printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
        char localaddrstr[SOCKADDR_STRLEN] = {0};
        char peeraddrstr[SOCKADDR_STRLEN] = {0};
        printf("[%s] <=> [%s]\n",
        SOCKADDR_STR(hio_localaddr(io), localaddrstr),
        SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

        printf("> %.*s", readbytes, (char*)buf);
    }
}

void handle_tar_shell(hio_t* io, void* buf, int readbytes)
{
    //按接收到的字符串进行比较
    char* str = (char*)buf;

    if(strncmp(str, "tar", 3) == 0)
    {
        printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
        char localaddrstr[SOCKADDR_STRLEN] = {0};
        char peeraddrstr[SOCKADDR_STRLEN] = {0};
        printf("[%s] <=> [%s]\n",
        SOCKADDR_STR(hio_localaddr(io), localaddrstr),
        SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

        printf("> %.*s", readbytes, (char*)buf);
    }
}

//读操作
static void tcp_recv(hio_t* io, void* buf, int readbytes) {
    printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("[%s] <=> [%s]\n",
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    char *tembuf = databuf; //设置临时缓存
    if (bytelen == 0) {
        memcpy(databuf, buf, readbytes);
        bytelen = readbytes;
    } else {
        memcpy(tembuf + bytelen, buf, readbytes);
        bytelen += readbytes;
    }
    
    netmt_command_t *comm = (netmt_command_t *)tembuf;

    netmt_command_t *sendbuf;
    HV_ALLOC_SIZEOF(sendbuf);

    if(comm->len > bytelen) {
        return;
    }

    while(bytelen != 0) {

        struct list_node *node;
        io_t *conn;

        switch (comm->commonid) {
            case NETMT_PING:
                if (comm->subcommonid == NETMT_SUCCESS) {
                    bzero(sendbuf, sizeof(*sendbuf));
                    memcpy(sendbuf, comm, sizeof(*comm));
                    hevent_set_priority(t_server.listenio, sendbuf);
                    // printf("server udp io set data!\n");
                }
                break;
            case NETMT_HEARTBEAT:
                if (comm->subcommonid == NETMT_SUCCESS) {
                    uint64_t cur_time = gethrtime_us();
                    int time = (cur_time - comm->last_send_ping_time) / 1000;
                    if (time > 0 && time < 15000) {
                        bzero(sendbuf, sizeof(*sendbuf));
                        memcpy(sendbuf, comm, sizeof(*sendbuf));
                        sendbuf->last_recv_pong_time = cur_time;
                        list_for_each(node, &t_server.clients) {
                            conn = list_entry(node, io_t, node);
                            if (conn->io->fd == sendbuf->fd) {
                                // list_entry(node, io_t, node)->count = 0;
                                sendbuf->len = sizeof(*sendbuf);
                                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                                bzero(sendbuf, sizeof(*sendbuf));
                            }
                        }
                    }
                }
                break;
            case NETMT_REGISTER:
                list_for_each(node, &t_server.clients) {
                    conn = list_entry(node, io_t, node);
                    if (conn->io->fd == comm->fd) {
                        // list_entry(node, io_t, node)->netmtclient;
                        memcpy(&(list_entry(node, io_t, node)->netmtclient), &comm->netmtclient, sizeof(netmtclient_s));
                        // printf("client register success\n");
                    }
                }
                t_server.commonid = comm->commonid;
                break;

            case NETMT_GET:
                if (comm->subcommonid == NETMT_FILE_OPEN) {
                    if (comm->file.localfile != NULL) {
                        char* file = &comm->file.localfile;
                        fp = fopen(file, "wb");
                        if (fp == NULL) {
                            perror("open|new file fail!");
                            return;
                        }else {
                            // printf("打开文件成功\n");
                            fsize = 0;
                        }
                    }
                }
                else if (comm->subcommonid == NETMT_DATA) {
                    // printf("写数据了\n");
                    hmutex_lock(&s_mutex);
                    int write_size = fwrite(comm->file.data, sizeof(char), comm->file.data_size, fp);
                    if (write_size > 0) {
                        // printf("客户端发送过来的文件数据大小:%d\n", comm->file.data_size);
                        // printf("写入文件数据的大小:%d\n", write_size);
                        fsize += write_size;
                        // perror("write data error!");
                        // return;
                    }
                    if (write_size < comm->file.data_size) {
                        perror("write data error!\n");
                        return;
                    }
                    hmutex_unlock(&s_mutex);
                }
                else if (comm->subcommonid == NETMT_GET_END) {
                    fclose(fp);
                    if (comm->file.byte_size == fsize) {
                        
                        bzero(sendbuf, sizeof(*sendbuf));
                        memcpy(sendbuf, comm, sizeof(*sendbuf));
                        sendbuf->subcommonid = NETMT_GET_SUCCESS;
                        
                        list_for_each(node, &t_server.clients) {
                            conn = list_entry(node, io_t, node);
                            if (conn->io->fd == sendbuf->fd) {
                                sendbuf->len = sizeof(*sendbuf);
                                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                                // bzero(sendbuf, sizeof(*sendbuf));
                            }
                        }
                        
                    }
                }
                break;

            case NETMT_SEND:
                if (comm->subcommonid == NETMT_SEND_SUCCESS) {
                    // vty_out(pvty, "%s\n", comm->file.str_end);
                    // return;
                }
                break;

            case NETMT_EXEC:
                    {
                        if (comm->subcommonid == NETMT_EXEC_SHELL_RESULT)
                        {
                            save_result_info_to_buffer(io, comm, sizeof(*comm));
                            handle_endflag(io, comm, sizeof(*comm));            
                        }
                        // else if (comm->subcommonid == NETMT_EXEC_SHELL_SUCCESS)
                        // {
                        //     handle_success(io, str, sizeof(*str));
                        // }
                        break;
                    }

            default:
                break;
        }
        
        // printf("comm->len=%d\n", comm->len);
        bytelen -= comm->len;
        // printf("bytelen=%d\n", bytelen);
        tembuf += comm->len;
        // printf("tem buf:%s\n", tembuf);
        comm = (netmt_command_t *)tembuf;
        // printf("tembuf->len=%d\n", comm->len);
        if (comm->len > bytelen) {
            memmove(databuf, tembuf, bytelen);
            break;
        }

    }
}

static void udp_recv(hio_t* io, void* buf, int readbytes) {
    printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("[%s] <=> [%s]\n",
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    netmt_command_t *comm = (netmt_command_t *)buf;

    netmt_command_t *sendbuf = NULL;
    HV_ALLOC_SIZEOF(sendbuf);
    bzero(sendbuf, sizeof(*sendbuf));
    memcpy(sendbuf, comm, sizeof(*comm));

    if (comm->commonid == NETMT_PING) {
        if (comm->subcommonid == NETMT_SUCCESS) {
            // char *str = "ping success";
            hevent_set_userdata(io, sendbuf);
            printf("server udp io set data!\n");

            // hevent_userdata(io);
        }
    }
}

static void netmt_accept(hio_t* io) {
    printf("on_accept connfd=%d\n", hio_fd(io));

    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("accept connfd=%d [%s] <= [%s]\n", hio_fd(io),
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
	
    hio_setcb_close(io, netmt_close);
    hio_setcb_read(io, tcp_recv);
    // hio_set_readbuf(io, comm, sizeof(netmt_command_t));
    // hio_setcb_write(io, on_send);
    // hio_read(io);
    hio_read_start(io);

    io_t *conn = NULL;
    HV_ALLOC_SIZEOF(conn);
    get_sockaddr(hio_peeraddr(io), conn, SOCKADDR_STRLEN);

    conn->io = io;
    conn->buffer_max_size = sizeof(conn->buffer);
    conn->buffer_current_size = 0;
    conn->flag = 0;

    hevent_set_userdata(io, conn);

    // hmutex_lock(&s_mutex);
    //加锁
    if (pthread_mutex_lock(&s_mutex) != 0) {
		perror("pthread_mutex_lock");
		exit(EXIT_FAILURE);
	}	
    list_add(&conn->node, &t_server.clients);
    // hmutex_unlock(&s_mutex);
    //解锁
	if (pthread_mutex_unlock(&s_mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

//tcp服务器
void* netmtTcp(void* arg) {
    addr_t *tcpaddr = (addr_t *)arg;
    
    // hloop_t* loop = hloop_new(HLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS);
    hloop_t *loop = hloop_new(0);
    hio_t* listenio = hloop_create_tcp_server(loop, tcpaddr->netmthost, tcpaddr->netmtport, netmt_accept);

    if (listenio == NULL) {
        perror("listenio error");
    }
    printf("listenfd=%d\n", hio_fd(listenio));
    // stdin use default readbuf
    t_server.loop = loop;
    t_server.listenio = listenio;
    srand(time(NULL));
    t_server.id = rand() % 1000000;
    list_init(&t_server.clients);
    // pthread_mutex_init(&s_mutex, NULL);

    hio_t* stdinio = hread(loop, 0, NULL, 0, netmt_send);
    if (stdinio == NULL) {
        perror("stdinio error");
    }

    hloop_run(loop);
    hloop_free(&loop);
}

//udp客户
void* netmtUdp(void* arg) {
    
    addr_t *tcpaddr = (addr_t *)arg;

    HV_MEMCHECK;

    hloop_t* loop = hloop_new(HLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS);

    hio_t* stdinio = hread(loop, 0, NULL, 0, netmt_send);
    if (stdinio == NULL) {
        perror("stdinio error");
    }

    udpio = hloop_create_udp_client(loop, "0.0.0.0", tcpaddr->netmtport);
    hio_read(udpio);
    if (udpio == NULL) {
        perror("Create Udp Client error");
    }
    // udpFD = udpio->fd;
    printf("udp fd=%d\n", hio_fd(udpio));
    // char sendbuf[BUFSIZ];
    // snprintf(sendbuf, BUFSIZ, "send UdpClient %s %d\n", tcpaddr->netmthost, tcpaddr->netmtport);

    // hio_setcb_write(io, netmt_send);
    // hio_setcb_close(udpio, netmt_close);
    hio_setcb_read(udpio, udp_recv);
    // hio_set_readbuf(udpio, recvbuf, BUFSIZ);

    hloop_run(loop);
    hloop_free(&loop);
}

// int GetAbsPath(char *szFile, char *szOutPath, int nBufLen);
int cmd_main(int argc, char **argv, CMD_INIT_OTHER pfun);

static void all_netmt_cmd_init() {
    netmt_servercmd_init();
}

int daemon_init(char *pname, int facility) {
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return -1;
	else if (pid)
		_exit(0);			/* parent terminates */

	/* child 1 continues... */

	if (setsid() < 0)			/* become session leader */
		return -1;

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) < 0)
		return -1;
	else if (pid)
		_exit(0);			/* child 1 terminates */

    umask (0027);
	return 0;				/* success */
}

void netmtprinthelp(){
    printf("\
    Usage: netmtserver [-bf]\n\
    Options:\n\
      -b       background run (default)\n\
      -f       foreground run\n\
    Examples: netmtserver\n\
              netmtserver -f\n");
}

int main(int argc, char** argv) {

    if (argc == 1) {
        daemon_init(argv[0], 0);
    }
    else if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0) {
            netmtprinthelp();
            return 0;
        }
        else if (strcmp(argv[1], "-b") == 0) {
            daemon_init(argv[0], 0);
        }
        else if (strcmp(argv[1], "-f") == 0) {

        }
    }

    hmutex_init(&s_mutex);

    if (argc == 0 || argv == NULL) {
        argc = 1;
        SAFE_ALLOC(argv, 2 * sizeof(char *));
        SAFE_ALLOC(argv[0], MAX_PATH);
        get_executable_path(argv[0], MAX_PATH);
    }

    main_ctx_t g_main_ctx;
    get_run_dir(g_main_ctx.run_dir, sizeof(g_main_ctx.run_dir));
    strncpy(g_main_ctx.program_name, hv_basename(argv[0]), sizeof(g_main_ctx.program_name));

    char logdir[MAX_PATH] = {0};
    snprintf(logdir, sizeof(logdir), "%s/../logs", g_main_ctx.run_dir);
    hv_mkdir(logdir);
    snprintf(g_main_ctx.confile, sizeof(g_main_ctx.confile), "%s/../etc/%s.conf", g_main_ctx.run_dir, g_main_ctx.program_name);
    snprintf(g_main_ctx.pidfile, sizeof(g_main_ctx.pidfile), "%s/../logs/%s.pid", g_main_ctx.run_dir, g_main_ctx.program_name);
    snprintf(g_main_ctx.logfile, sizeof(g_main_ctx.confile), "%s/../logs/%s.log", g_main_ctx.run_dir, g_main_ctx.program_name);
    hlog_set_file(g_main_ctx.logfile);
    //日志文件保存3天
    hlog_set_remain_days(3);

    static pthread_t tcp_thread_id, udp_thread_id;
	
    char netmtfile[256] = "\0";
    char *inifile = "../etc/netmtserver.ini";
    if (!GetAbsPath(inifile, netmtfile, sizeof(netmtfile))) {
        printf("\nGet cfg-file failed! file='%s'\n", inifile);
    }
    
    const char host[SOCKADDR_STRLEN] = {0};
    int port;
   
    if (!CGetValueString(netmtfile, "server", "serverhost", "0.0.0.0", host, "")) {
        printf("server:host error!");
    }
    if (!CGetValueInt(netmtfile, "server", "serverport", 1234, &port, "")) {
        printf("server:port error!");
    }

    addr_t *tcp_addr = NULL;
    HV_ALLOC_SIZEOF(tcp_addr);
    bzero(tcp_addr, sizeof(*tcp_addr));
    strcpy(tcp_addr->netmthost, host);
    // printf("ip copy success\n");
    tcp_addr->netmtport = port;

	if (pthread_create(&tcp_thread_id, NULL, (void*)netmtTcp, (void*)tcp_addr) == -1) {
        printf("creat Tcp error\n");
        return -1;
    }
	if (pthread_create(&udp_thread_id, NULL, (void*)netmtUdp, (void*)tcp_addr) == -1) {
        printf("creat Udp error\n");
        return -1;
    }
	
    cmd_main(argc, argv, all_netmt_cmd_init);

	pthread_join(tcp_thread_id, NULL);
	pthread_join(udp_thread_id, NULL);


    return 0;
}
