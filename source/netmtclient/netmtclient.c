#include "hv.h"
#include "hloop.h"
#include "event/hevent.h"
#include "netmt.h"
#include "inifile.h"
#include "netmtcommand.h"

#define RECV_BUFSIZE    8192
static char recvbuf[RECV_BUFSIZE];

// for stdin
// hio_t*      tcpinio = NULL;
// for socket
hio_t*      sockio = NULL;
hio_t *udpio = NULL;
hio_t *tcpio = NULL;
netmtclient_s *netmtclient;

char databuf[DATABUF];
int bytelen = 0;
FILE *fp;
struct stat g_statbuf;
static int filebytes;

static hmutex_t s_mutex;

void send_heartbeat(hio_t* io) {
    // netmt_command_t *sendbuf = (netmt_command_t *)hevent_userdata(io);
    netmt_command_t *sendbuf = NULL;
    HV_ALLOC_SIZEOF(sendbuf);
    bzero(sendbuf, sizeof(*sendbuf));
    sendbuf->commonid = NETMT_HEARTBEAT;
    sendbuf->subcommonid = NETMT_SUCCESS;
    sendbuf->len = sizeof(*sendbuf);
    sendbuf->fd = hio_fd(io);
    sendbuf->last_send_ping_time = gethrtime_us();
    hio_write(io, sendbuf, sizeof(*sendbuf));
    HV_FREE(sendbuf);
    // if (sendbuf->last_recv_pong_time < sendbuf->last_send_ping_time) {
    //     printf("client connect timeout!\n");
    //     hio_close(io);
    //     HV_FREE(sendbuf);
    // } else {
    //     static char buf[] = "PING\r\n";
    //     memcpy(sendbuf->cmdstr, buf, sizeof(buf));
    //     sendbuf->len = sizeof(*sendbuf);
    //     sendbuf->fd = hio_fd(io);
    //     sendbuf->last_send_ping_time = gethrtime_us();
    //     hio_write(io, sendbuf, sizeof(*sendbuf));
    // }
}

//	获取处理命令的管道文件流
FILE *get_fstream(void *cmd)
{
    char *str = (char *)cmd;
    FILE *fstream;

    if ((fstream = popen(str, "r")) == NULL)
    {
        fprintf(stderr, "execute command failed: %s", strerror(errno));
        return NULL;
    }
    else
    {
        return fstream;
    }
}

int exec_command(void * cmd)
{
	char* str = (char *)cmd;
	FILE* fstream = NULL;
    char success[64] = "shell exec success.";
	char buff[RECV_BUFSIZE];
	memset(buff, 0, sizeof(buff));
    netmt_command_t tmp_cmd;
    memset(&tmp_cmd, 0, sizeof(tmp_cmd));
    strcpy(tmp_cmd.cmdstr, success);
    tmp_cmd.commonid = NETMT_EXEC;
    tmp_cmd.subcommonid = NETMT_EXEC_SHELL_SUCCESS;
    tmp_cmd.len = sizeof(tmp_cmd);
    strcpy(buff, str);

	fstream = get_fstream(str);
	if(fstream == NULL)
	{
		printf("error, cannot popen cmd, %s\n",str);
		return -1;
	}
	
    printf("%c",'\n');
	while((fgets(buff, sizeof(buff), fstream)) != NULL)
	{
		if(buff[strlen(buff - 1)] == '\n' || buff[strlen(buff) - 1] == '\r')
		{
			buff[strlen(buff) - 1] = '\0';
		}
		
		printf("%s", buff);
	}

    hio_write(sockio, &tmp_cmd, sizeof(netmt_command_t));
	pclose(fstream);
	return 0;
}

int exec_command_with_return(void * cmd)
{
	char* str = (char *)cmd;
	FILE* fstream = NULL;
	char buff[POPEN_SIZE] = {0};
    char tmpbuffer[POPEN_SIZE] = {0};

    strcpy(buff, str);
	fstream = get_fstream(str);
	if(fstream == NULL)
	{
		printf("error, cannot popen cmd, %s\n",str);
		return -1;
	}

    printf("%c", '\n');
	while((fgets(buff, sizeof(buff), fstream)) != NULL)
	{
		if(buff[strlen(buff) - 1] == '\n' || buff[strlen(buff) - 1] == '\r')
		{
			//buff[strlen(buff) - 1] = '\0';
		}
	
		//printf("%s", buff);
        strcat(tmpbuffer, buff);   
        strcat(tmpbuffer, "\r");

        if (strlen(tmpbuffer) > POPEN_SIZE-10) {
            break;
        }
	}
    printf("%s", "\n");

    //保存所有shell命令结果
    netmt_command_t tmp_cmd;
    memset(&tmp_cmd, 0, sizeof(tmp_cmd));
    strcpy(tmp_cmd.total_result, tmpbuffer);
    printf("total_result : \n%s\n", tmp_cmd.total_result);
    tmp_cmd.commonid = NETMT_EXEC;
    tmp_cmd.subcommonid = NETMT_EXEC_SHELL_RESULT;
    tmp_cmd.len = sizeof(tmp_cmd);
    tmp_cmd.flag = 1;
    hio_write(sockio, &tmp_cmd, sizeof(tmp_cmd));

	pclose(fstream);
	return 0;
}

void tcp_recv(hio_t* io, void* buf, int readbytes) {
    printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    
    char *tembuf = databuf; //设置临时缓存
    if (bytelen == 0) {
        memcpy(databuf, buf, readbytes);
        bytelen = readbytes;
    } else {
        memcpy(tembuf + bytelen, buf, readbytes);
        bytelen += readbytes;
    }

    netmt_command_t* comm = (netmt_command_t*)tembuf;
    if(comm->len > bytelen)
        return;

    netmt_command_t *sendbuf = NULL;
    HV_ALLOC_SIZEOF(sendbuf);
    // printf("NETMT_SEND=%d\n", comm->commonid);
    // printf("NETMT_CLIETN=%d\n", comm->subcommonid);
    while (bytelen != 0) {
        // printf("while bytelen=%d\n", bytelen);
        switch (comm->commonid) {
            case NETMT_HEARTBEAT:
                if (comm->subcommonid == NETMT_SUCCESS) {
                    bzero(sendbuf, sizeof(*sendbuf));
                    memcpy(sendbuf, comm, sizeof(*sendbuf));
                    if (sendbuf->last_recv_pong_time < sendbuf->last_send_ping_time) {
                        printf("client connect timeout!\n");
                        hio_close(io);
                        HV_FREE(sendbuf);
                    }
                }
                break;

            case NETMT_REGISTER:
                bzero(sendbuf, sizeof(*sendbuf));
                memcpy(sendbuf, comm, sizeof(*sendbuf));

                char netmtfile[256] = "\0";
                char *inifile = "../etc/netmtclient.ini";
                if (!GetAbsPath(inifile, netmtfile, sizeof(netmtfile))) {
                    printf("\nGet cfg-file failed! file='%s'\n", inifile);
                }

                HV_ALLOC_SIZEOF(netmtclient);
                const char host[SOCKADDR_STRLEN] = {0};
                int port;
                int id;
                const char area[LEN] = {0};
                const char type[LEN] = {0};

                if (!CGetValueString(netmtfile, "client", "clienthost", "0.0.0.0", host, "")) {
                    printf("client:host error!");
                }
                if (!CGetValueInt(netmtfile, "client", "clientport", 1234, &port, "")) {
                    printf("client:port error!");
                }
                if (!CGetValueInt(netmtfile, "client", "id", NULL, &id, "")) {
                    printf("client:port error!");
                }
                if (!CGetValueString(netmtfile, "client", "area", "", area, "")) {
                    printf("client:area error!");
                }
                if (!CGetValueString(netmtfile, "client", "subarea", "", type, "")) {
                    printf("client:type error!");
                }

                strcpy(netmtclient->client_addr.netmthost, host);
                netmtclient->client_addr.netmtport = port;
                netmtclient->id = id;
                strcpy(netmtclient->area, area);
                strcpy(netmtclient->type, type);

                memcpy(&sendbuf->netmtclient, netmtclient, sizeof(netmtclient_s));
                hio_write(io, sendbuf, sizeof(*sendbuf));
                HV_FREE(netmtclient);

                break;

            case NETMT_GET:
                if (comm->subcommonid == NETMT_CLIENT) {
                    // printf("\r\r发送文件中.....\n");
                    // printf("NETMT_GET=%d\n", comm->commonid);
                    // int fd = comm->fd;
                    if (comm->file.remotefile != NULL) {
                        // strncmp(file, &comm->file.remotefile, FILE_LEN);
                        char* file = &comm->file.remotefile;
                        // printf("远端文件:%s\n", file);
                        // printf("remote file:%s\n", &comm->file.remotefile);
                        fp = fopen(file, "rb");
                        if (fp == NULL) {
                            printf("File:\t%s can not open to read!\n", file);
                            return;
                        }else {
                            filebytes = 0;
                            // bzero(comm, sizeof(*comm));
                            bzero(sendbuf, sizeof(*sendbuf));
                            memcpy(sendbuf, comm, sizeof(*sendbuf));
                            sendbuf->subcommonid = NETMT_FILE_OPEN;
                            sendbuf->len = sizeof(*sendbuf);
                            hio_write(io, sendbuf, sizeof(*sendbuf));
                        }
                    }
                    sendbuf->subcommonid = NETMT_DATA;
                    // printf("发送文件数据给服务器\n");

                    int file_block_length = 0;
                    
                    hmutex_lock(&s_mutex);
                    while ((file_block_length = fread(&sendbuf->file.data, sizeof(char), BUFFER_SIZE, fp)) > 0) {
                        filebytes += file_block_length;
                        // bzero(sendbuf, sizeof(*sendbuf));
                        // memcpy(sendbuf, comm, sizeof(*sendbuf));

                        sendbuf->file.data_size = file_block_length;
                        // printf("读取文件数据的大小:%d\n", sendbuf->file.data_size);
                        // comm->commonid = NETMT_DATA;
                        // printf("发送数据.....\n");
                        // bzero(sendbuf, sizeof(*sendbuf));
                        // memcpy(sendbuf, comm, sizeof(*sendbuf));
                        sendbuf->len = sizeof(*sendbuf);
                        hio_write(io, sendbuf, sizeof(*sendbuf));
                        bzero(&sendbuf->file.data, BUFFER_SIZE);
                        // printf("file data init\n");
                        // bzero(comm, sizeof(*comm));
                    }
                    hmutex_unlock(&s_mutex);

                    sendbuf->subcommonid = NETMT_GET_END;
                    // printf("发送文件完毕\n");
                    // printf("文件结构大小:%d\n", sizeof(sendbuf->file));
                    sendbuf->file.byte_size = filebytes;
                    // printf("发送总字节数:%d\n", filebytes);

                    sendbuf->len = sizeof(*sendbuf);
                    hio_write(io, sendbuf, sizeof(*sendbuf));
                    
                    fclose(fp);
                    // printf("close file\n");

                    HV_FREE(sendbuf); //释放申请的空间
                } 
                else if (comm->subcommonid == NETMT_GET_END) {
                    // printf("%s\n", comm->file.str_end);
                    // return;
                }
                break;

            case NETMT_SEND:
                if (comm->subcommonid == NETMT_FILE_OPEN) {
                    if (comm->file.remotefile != NULL) {
                        char *file = &comm->file.remotefile;
                        // printf("remote file:%s\n", file);
                        fp = fopen(file, "wb");
                        if (fp == NULL) {
                            perror("open|new file fail!");
                            return;
                        }else {
                            printf("open file success\n");
                            filebytes = 0;
                        }
                    }
                }
                else if (comm->subcommonid == NETMT_DATA) {
                    // printf("write data!\n");
                    hmutex_lock(&s_mutex);
                    int write_size = fwrite(comm->file.data, sizeof(char), comm->file.data_size, fp);
                    filebytes += write_size;
                    // if (write_size > 0) {
                    //     printf("server send file data size:%d\n", comm->file.data_size);
                    //     printf("write file data size:%d\n", write_size);
                    // }
                    if (write_size < comm->file.data_size) {
                        perror("write data error!\n");
                        return;
                    }
                    hmutex_unlock(&s_mutex);
                }
                else if (comm->subcommonid == NETMT_SEND_END) {
                    fclose(fp);
                    // printf("data write over, close file\n");
                    // printf("server send bytes:%d\n", comm->file.byte_size);
                    // printf("client write bytes:%d\n", filebytes);
                    if (comm->file.byte_size == filebytes) {
                        // printf("client send file get success to server\n");
                        bzero(sendbuf, sizeof(*sendbuf));
                        memcpy(sendbuf, comm, sizeof(*sendbuf));
                        sendbuf->subcommonid = NETMT_SEND_SUCCESS;

                        hio_write(io, sendbuf, sizeof(*sendbuf));
                    }
                }
                break;

            case NETMT_EXEC:
                    {
                        char* str = comm->cmdstr;
                        char origin[1024] = {0};
                        int len = 1024;
                        strcpy(origin, comm->cmdstr);

                        //printf(">%.*s\n", readbytes, str);
                        char eol = str[len-1];
                        if (eol == '\n' || eol == '\r') {
                            if (len > 1 && str[len-2] == '\r' && eol == '\n') {
                                // have been CRLF
                            }
                            else {
                                ++len;
                                str[len - 2] = '\r';
                                str[len - 1] = '\n';
                            }
                        }
                        else if (strncmp(str, "ifconfig cmd", 12) == 0) {
                            char* strptr = "ifconfig";
                            exec_command(strptr);
                        } else if (strncmp(str, "tar ", 4) == 0) {
                            exec_command_with_return(origin);
                        } else {
                            //exec_command(origin);        
                            exec_command_with_return(origin);
                            // printf("command end\n");
                        }
                    }
                    break;

            default:

                break;
        }

        // printf("comm->len=%d\n", comm->len);
        bytelen -= comm->len;
        // printf("bytelen=%d\n", bytelen);
        tembuf += comm->len;
        comm = (netmt_command_t *)tembuf;
        // printf("tembuf->len=%d\n", comm->len);
        if (comm->len > bytelen) {
            memmove(databuf, tembuf, bytelen);
            break;
        }
    }

    fflush(stdout);
}

void tcp_stdin(hio_t* io, void* buf, int readbytes) {

    char* str = (char*)buf;

    //判断输入命令的长度
    
    if (strncmp(str, "start", 5) == 0) {
        printf("call hio_read_start\n");
        hio_read_start(sockio);
        return;
    }
    else if (strncmp(str, "stop", 4) == 0) {
        printf("call hio_read_stop\n");
        hio_read_stop(sockio);
        return;
    }
    else if (strncmp(str, "close", 5) == 0) {
        printf("call hio_close\n");
        hio_close(sockio);
        return;
    }
    else if (strncmp(str, "quit", 4) == 0) {
        printf("call hloop_stop\n");
        hloop_stop(hevent_loop(io));
        return;
    }

}

void udp_stdin(hio_t* io, void* buf, int readbytes) {

    char* str = (char*)buf;

    //判断输入命令的长度
    
    if (strncmp(str, "start", 5) == 0) {
        printf("call hio_read_start\n");
        hio_read_start(udpio);
        return;
    }
    else if (strncmp(str, "stop", 4) == 0) {
        printf("call hio_read_stop\n");
        hio_read_stop(udpio);
        return;
    }
    else if (strncmp(str, "close", 5) == 0) {
        printf("call hio_close\n");
        hio_close(udpio);
        return;
    }
    else if (strncmp(str, "quit", 4) == 0) {
        printf("call hloop_stop\n");
        hloop_stop(hevent_loop(io));
        return;
    }

}

void netmt_close(hio_t* io) {
    //printf("on_close fd=%d error=%d\n", hio_fd(io), hio_error(io));
    hio_del(io, HV_READ);
}

void netmt_connect(hio_t* io) {
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("udp connfd=%d [%s] <= [%s]\n", hio_fd(io),
            SOCKADDR_STR(hio_localaddr(io), localaddrstr),
            SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    netmt_command_t *iobuf = NULL;
    HV_ALLOC_SIZEOF(iobuf);
    bzero(iobuf, sizeof(*iobuf));
    hevent_set_userdata(io, iobuf);
    hio_read_start(io);
    // uncomment to test heartbeat
    //hio_set_heartbeat(sockio, 3000, send_heartbeat);//时间间隔4分钟
    hio_set_heartbeat(io, HeartTime, send_heartbeat);
}

void* netmtTcp(void* arg) {
    addr_t *tcpaddr = (addr_t *)arg;

    HV_MEMCHECK;

    hloop_t* loop = hloop_new(HLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS);

    // stdin use default readbuf
    hio_t* stdinio = hread(loop, 0, NULL, 0, tcp_stdin);
    if (stdinio == NULL) {
        perror("-20");
    }

    sockio = hloop_create_tcp_client(loop, tcpaddr->netmthost, tcpaddr->netmtport, netmt_connect);
    if (sockio == NULL) {
        perror("sockio error");
    }

    printf("sockfd=%d\n", hio_fd(sockio));
    hio_setcb_close(sockio, netmt_close);
    hio_setcb_read(sockio, tcp_recv);
    hio_set_readbuf(sockio, recvbuf, RECV_BUFSIZE);

    // hevent_set_priority(sockio, netmtclient);

    hloop_run(loop);
    hloop_free(&loop);
}

void* netmt_setTcp(void* arg) {
    netmt_command_t *comm = (netmt_command_t *)arg;

    HV_MEMCHECK;

    hloop_t* loop = hloop_new(HLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS);

    // stdin use default readbuf
    hio_t* stdinio = hread(loop, 0, NULL, 0, tcp_stdin);
    if (stdinio == NULL) {
        //配置错误日志
        main_ctx_t g_main_ctx;
        get_run_dir(g_main_ctx.run_dir, sizeof(g_main_ctx.run_dir));
        strncpy(g_main_ctx.program_name, hv_basename("bin/"), sizeof(g_main_ctx.program_name));
        char logdir[MAX_PATH] = {0};
        //日志存放路径
        snprintf(logdir, sizeof(logdir), "%s/../logs", g_main_ctx.run_dir);
        //创建日志目录
        hv_mkdir(logdir);
        snprintf(g_main_ctx.logfile, sizeof(g_main_ctx.logfile), "%s/../logs/%s.log", g_main_ctx.run_dir, g_main_ctx.program_name);
    
        hlog_set_file(g_main_ctx.logfile);
        hlog_set_level(LOG_LEVEL_ERROR);
        hlog_set_remain_days(3);
		
		//打印错误日志
        hloge("Client failed to connect to server!");
        perror("-20");
    }

    sockio = hloop_create_tcp_client(loop, comm->netmtclient.client_addr.netmthost, comm->netmtclient.client_addr.netmtport, netmt_connect);
    if (sockio == NULL) {
        perror("sockio erroe");
    }

    printf("sockfd=%d\n", hio_fd(sockio));
    comm->subcommonid = NETMT_SUCCESS;
    comm->len = sizeof(*comm);
    hio_write(sockio, comm, sizeof(*comm));
    // printf("client tcp reset success!\n");

    hio_setcb_close(sockio, netmt_close);
    hio_setcb_read(sockio, tcp_recv);
    hio_set_readbuf(sockio, recvbuf, RECV_BUFSIZE);
    // hevent_set_priority(sockio, netmtclient);
    hloop_run(loop);
    hloop_free(&loop);
}

void udp_recv(hio_t* io, void* buf, int readbytes) {
    printf("udp_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);

    netmt_command_t *comm = (netmt_command_t *)buf;
    if (comm->commonid == NETMT_PING) {
        // netmt_command_t *udpsend = NULL;
        // HV_ALLOC_SIZEOF(udpsend);
        // bzero(udpsend, sizeof(*udpsend));
        // memcpy(udpsend, comm, sizeof(*comm));
        // hloop_free(&sockio->loop);
        //重启tcp

        static pthread_t re_tcpid;
        if (pthread_create(&re_tcpid, NULL, (void*)netmt_setTcp, (void*)comm) == -1) {
            printf("reset create Tcp error\n");
            return;
        }
        // printf("client set tcp information send!\n");

        pthread_join(re_tcpid, NULL);

        // comm->subcommonid = NETMT_PING_SUCCESS;
        // hio_write(udpio, comm, sizeof(*comm));
    }
}

void* netmtUdp(void* arg) {

    addr_t *udpaddr = (addr_t *)arg;

    hloop_t* loop = hloop_new(0);

    // hio_t *stdinio = hread(loop, 0, NULL, 0, udp_stdin);
    // if (stdinio == NULL) {
    //     return;
    // }

    udpio = hloop_create_udp_server(loop, udpaddr->netmthost, udpaddr->netmtport);
    // printf("udp io success!\n");

    if (udpio == NULL) {
        perror("Create Udp Server error");
    }

    printf("udp fd=%d\n", hio_fd(udpio));
    hio_setcb_read(udpio, udp_recv);
    hio_read(udpio);

    hloop_run(loop);
    hloop_free(&loop);
}

// int cmd_main(int argc, char **argv, CMD_INIT_OTHER pfun);

// static void all_netmt_cmd_init() {
//     netmt_clientcmd_init();
//     // printf("cmd end!\n");
// }

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
    Usage: netmtclient [-bf]\n\
    Options:\n\
      -b       background run (default)\n\
      -f       foreground run\n\
    Examples: netmtclient\n\
              netmtclient -f\n");
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
    char *inifile = "../etc/netmtclient.ini";
    if (!GetAbsPath(inifile, netmtfile, sizeof(netmtfile))) {
        printf("\nGet cfg-file failed! file='%s'\n", inifile);
    }
    
    char serverhost[SOCKADDR_STRLEN] = {0};
    char clienthost[SOCKADDR_STRLEN] = {0};
    int serverport;
    int clientport;
   
    if (!CGetValueString(netmtfile, "server", "serverhost", "0.0.0.0", serverhost, "")) {
        printf("server:host error!");
    }
    if (!CGetValueInt(netmtfile, "server", "serverport", 1234, &serverport, "")) {
        printf("server:port error!");
    }
    if (!CGetValueString(netmtfile, "client", "clienthost", "0.0.0.0", clienthost, "")) {
        printf("client:host error!");
    }
    if (!CGetValueInt(netmtfile, "client", "clientport", 1234, &clientport, "")) {
        printf("client:port error!");
    }

    addr_t *tcp_addr = NULL;
    HV_ALLOC_SIZEOF(tcp_addr);
    bzero(tcp_addr, sizeof(*tcp_addr));
    strcpy(tcp_addr->netmthost, serverhost);
    tcp_addr->netmtport = serverport;

    addr_t *udp_addr = NULL;
    HV_ALLOC_SIZEOF(udp_addr);
    bzero(udp_addr, sizeof(*udp_addr));
    strcpy(udp_addr->netmthost, clienthost);
    udp_addr->netmtport = clientport;

    if (pthread_create(&tcp_thread_id, NULL, (void*)netmtTcp, (void*)tcp_addr) == -1) {
        printf("creat Tcp error\n");
        return -1;
    }

    if (pthread_create(&udp_thread_id, NULL, (void*)netmtUdp, (void*)udp_addr) == -1) {
        printf("creat Udp error\n");
        return -1;
    }

    pthread_join(udp_thread_id, NULL);
	pthread_join(tcp_thread_id, NULL);

    // cmd_main(argc, argv, all_netmt_cmd_init);

    return 0;
}
