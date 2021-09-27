// #include "zebra.h"
#include "netmtcommand.h"
#include "netmt.h"
#include "memory.h"

extern server_t t_server;
extern io_t t_io;
extern int udpFD;
extern hio_t *udpio;
addr_t client_addr;
static hmutex_t s_mutex;

conf_ctx_t g_conf_ctx;

netmt_command_t comm;
FILE *fp;

int filebytes;
// int fsize = 0;
// struct list_node *node;
//版本
char *default_version =
    "\r\nHello, this is NetMt Tool(version V0.0.1).\r\nCopyright 2021-2022 .\r\n";

// short options
static const char options[] = "hvc:ts:dp:o";

static const option_t long_options[] = {
    {'h', "help", NO_ARGUMENT},
    {'v', "version", NO_ARGUMENT},
    {'c', "confile", REQUIRED_ARGUMENT},
    {'t', "test", NO_ARGUMENT},
    // {'s', "signal",     REQUIRED_ARGUMENT},
    {'d', "daemon", NO_ARGUMENT},
    {'p', "port", REQUIRED_ARGUMENT},
    {'o', "command", OPTIONAL_ARGUMENT},
    {'s', "send", REQUIRED_ARGUMENT}};

static const char detail_options[] = "\r\nnetmt help                 Print this information"
                                     "\r\nshow version              Print version"
                                     "\r\n-c|--confile <confile>    Set configure file, default etc/{program}.conf"
                                     "\r\n-t|--test                 Test configure file and exit"
                                     "\r\n-d|--daemon               Daemonize"
                                     "\r\n-p|--port <port>          Set listen port"
                                     "\r\n-o|--basic command        common command: ls, mkdir, rm, touch, cat, vim"
                                     "\r\n-s|--send command         send (fd) string(like ls, rm ...) or only invalid string"
                                     "\r\nshow netmt client    show client information ";

static const char command[] = "\nUsage:"
                              "\n-o ls"
                              "\nmkdir"
                              "\nrmdir"
                              "\ntouch"
                              "\ncat"
                              "\nvim";

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

int exec_command(void *cmd)
{
    char *str = (char *)cmd;
    FILE *fstream;

    char buff[BUFSIZ];
    memset(buff, 0, sizeof(buff));

    fstream = get_fstream(str);
    if (fstream == NULL)
    {
        printf("error, cannot popen cmd, %s\n", str);
        return -1;
    }

    while ((fgets(buff, sizeof(buff), fstream)) != NULL)
    {
        if (buff[strlen(buff - 1)] == '\n' || buff[strlen(buff) - 1] == '\r')
        {
            buff[strlen(buff) - 1] = '\0';
        }

        printf("%s", buff);
    }

    pclose(fstream);
    return 0;
}

//向客户端发送命令，群发
static void send_total_string(void* cmd, server_t* t_cli) {
    struct list_node *node;
    io_t *conn;
    int connfd[10] = {0};
    int count = 0;
    netmt_command_t* comm = (netmt_command_t*)cmd;
    //hmutex_lock(&s_mutex);

    //strcpy(string, comm->cmdstr);

    list_for_each(node, &t_cli->clients) {
        conn = list_entry(node, io_t, node);

        if(conn->io) 
        {
            if (conn->io->fd != 0) 
            {
                connfd[count] = conn->io->fd;
                printf("\ncount = %d, fd = %d\n", count, connfd[count]);
                count++;
                
                comm->len = sizeof(*comm);
                hio_write(conn->io, comm, sizeof(netmt_command_t));
            }
        }
    }
    printf("send_total_string end\n");

    //hmutex_unlock(&s_mutex);
}

//根据FD向指定的客户端发送命令
static void send_single_fd(int fd, void* cmd, server_t* t_cli) {
    struct list_node *node;
    io_t *conn;
    netmt_command_t* comm = (netmt_command_t*) cmd;

    //strcpy(string, comm->cmdstr);
    list_for_each(node, &t_cli->clients) {
        conn = list_entry(node, io_t, node);

        if(conn->io)
        {
            if (conn->io->fd == fd) 
            {
                comm->len = sizeof(*comm);
                hio_write(conn->io, comm, sizeof(netmt_command_t));
                //break;
            }
        }
    }
}

static void print_info_using_vty_format(char* string, char eol, struct vty* pvty)
{   
    char* current = strchr(string, eol);
    char* last = strrchr(string, eol);
    char* tmp = string;
    while(current != last)
    {
        *current = '\0';
        vty_out(pvty, "%s\r", tmp);
        //printf("%s\n", tmp);
        tmp = current+1;
        current = strchr(tmp, eol);
    }
    vty_out(pvty, "%s\r", tmp);
    //printf("%s\n", tmp);
}

//顺序打印所有客户端的返回信息
void print_all_client_return_info(server_t* t_cli, struct vty* pvty)
{
    struct list_node* node;
    io_t* conn;

    //sleep(3);
    if (pthread_mutex_lock(&s_mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
    list_for_each(node, &t_cli->clients)
    {
        conn = list_entry(node, io_t, node);
        if (conn->flag == 1)
        {
            vty_out(pvty, "\r\n******************************\r\n");
            vty_out(pvty, "client (id area type) = (%d %s %s)\r\n",conn->netmtclient.id, 
                            conn->netmtclient.area, conn->netmtclient.type);
           
            print_info_using_vty_format(conn->buffer, '\r', pvty);

            vty_out(pvty, "******************************\r\n\n");

            memset(conn->buffer, '\0', sizeof(conn->buffer));
            conn->flag = 0;
            conn->buffer_current_size = 0;           
        }

    }
    if (pthread_mutex_unlock(&s_mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }

}

void printf_help(struct vty *pvty)
{
    vty_out(pvty, "Usage: %s\r\n", options);
    vty_out(pvty, "Options:%s\r\n", detail_options);
}

void show_help(struct vty *pvty)
{
    printf_help(pvty);
    return;
}

void show_version(struct vty *pvty)
{
    vty_out(pvty, "%s", default_version);
    return;
}

void show_netmt_addr(struct vty* pvty) {
    struct list_node *node;
    io_t *conn;
    vty_out(pvty, "\raddress\t\tFD\r\n");
    list_for_each(node, &t_server.clients)
    {
        conn = list_entry(node, io_t, node);
        vty_out(pvty, "\r%s:%d\t\t%d\r\n", conn->netmtaddr.netmthost, conn->netmtaddr.netmtport, conn->io->fd);
    }
    return;
}

void show_netmt_client_status(struct vty *pvty)
{
    struct list_node *node;
    io_t *conn;
    //遍历整个链表
    vty_out(pvty, "\rid\taddress\t\t\tFD\ttype\tarea\r\n");
    list_for_each(node, &t_server.clients)
    {
        conn = list_entry(node, io_t, node);
        // netmtclient_s *netmtclient = (netmtclient_s *)hevent_userdata(conn->io);
        vty_out(pvty, "\r%d\t%s:%d\t%d\t%s\t%s\r\n", conn->netmtclient.id, conn->netmtclient.client_addr.netmthost, conn->netmtclient.client_addr.netmtport, conn->io->fd, conn->netmtclient.type, conn->netmtclient.area);
    }
    return;
}

int config_write_show_netmt_cmd(struct vty *vty)
{
    return CMD_SUCCESS;
}

struct cmd_node netmt_cmd_node = {
    VTY_NODE,
    "%s(CONFIG-LINE)# ",
};

DEFUN(show_netmt_help, show_netmt_help_cmd, "netmt help", SHOW_STR "command help\n")
{
    show_help(vty);
    return CMD_SUCCESS;
}

DEFUN(show_netmt_version, show_netmt_version_cmd, "show netmt version", SHOW_STR "show netmt version\n")
{
    show_version(vty);
    return CMD_SUCCESS;
}

DEFUN(show_status_netmt, show_status_netmt_cmd, "show netmt client", SHOW_STR "show netmt client information\n")
{
    show_netmt_client_status(vty);
    return CMD_SUCCESS;
}

DEFUN(show_netmt_client_addr, show_netmt_client_addr_cmd, "show client addr", SHOW_STR "show netmt client addr\n")
{
    show_netmt_addr(vty);
    return CMD_SUCCESS;
}

DEFUN(send_netmt, send_netmt_cmd, "send [FILENAME] to remote (all|group|type|single) [CLIENT] [FILENAME]", "send filepath to remote all|client(ip) filepath\n")
{
    // netmt_command_t comm;
    bzero(&comm, sizeof(comm));

    netmt_command_t *sendbuf = NULL;
    HV_ALLOC_SIZEOF(sendbuf);

    struct list_node *node;
    // // struct stat statbuf;
    io_t *conn;
    char *localfile;
    char *remotefile;
    char *cwd;
    if (argc == 0) {
        vty_out(vty, "invalid command.%s", VTY_NEWLINE);
        return CMD_WARNING;
    }
    //获取本地文件绝对路径
    if (!IS_DIRECTORY_SEP(*argv[0])) {
        cwd = getcwd(NULL, MAXPATHLEN);
        localfile = XMALLOC(MTYPE_TMP, strlen(cwd) + strlen(argv[0]) + 2);
        sprintf(localfile, "%s/%s", cwd, argv[0]);
    }
    else {
        localfile = argv[0];
    }
    //获取远端文件绝对路径
    if (strncmp(argv[1], "all", 3) == 0) {
        if (!IS_DIRECTORY_SEP(*argv[2])) {
        cwd = getcwd(NULL, MAXPATHLEN);
        remotefile = XMALLOC(MTYPE_TMP, strlen(cwd) + strlen(argv[2]) + 2);
        sprintf(remotefile, "%s/%s", cwd, argv[2]);
        }else {
            remotefile = argv[2];
        }
    }else {
        if (!IS_DIRECTORY_SEP(*argv[3])) {
        cwd = getcwd(NULL, MAXPATHLEN);
        remotefile = XMALLOC(MTYPE_TMP, strlen(cwd) + strlen(argv[3]) + 2);
        sprintf(remotefile, "%s/%s", cwd, argv[3]);
        }else {
            remotefile = argv[3];
        }
    }

    strncpy(comm.file.localfile, localfile, FILE_LEN);
    comm.file.localfile[FILE_LEN - 1] = '\0';
    vty_out(vty, "\rlocalfile:%s\r\n", comm.file.localfile);

    strncpy(comm.file.remotefile, remotefile, FILE_LEN);
    comm.file.remotefile[FILE_LEN - 1] = '\0';
    vty_out(vty, "\rremotefile:%s\r\n", comm.file.remotefile);

    fp = fopen(localfile, "rb");
    if (fp == NULL) {
        vty_out(vty, "File:\t%s can not open to read!\n", localfile);
        return -1;
    }
    vty_out(vty, "\r\nfile:%s open success\r\n", localfile);
    comm.commonid = NETMT_SEND;
    comm.subcommonid = NETMT_FILE_OPEN;

    bzero(sendbuf, sizeof(*sendbuf));
    memcpy(sendbuf, &comm, sizeof(*sendbuf));
    sendbuf->len = sizeof(*sendbuf);
    if (strncmp(argv[1], "all", 3) == 0) {

        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (conn != NULL) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                vty_out(vty, "\r\n******************************\r\n");
                vty_out(vty, "\rsend to %s client\r\n", argv[1]);
            }
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_DATA;
        // vty_out(vty, "send file data to client\r\n");
        int file_block_length = 0;
        //遍历整个链表
        hmutex_lock(&s_mutex);
        while ((file_block_length = fread(&sendbuf->file.data, sizeof(char), BUFFER_SIZE, fp)) > 0) {
            filebytes += file_block_length;
            sendbuf->file.data_size = file_block_length;
            // printf("read file data size:%d\n", sendbuf->file.data_size);
            // vty_out(vty, "send data to all....\r\n");
            sendbuf->len = sizeof(*sendbuf);
            list_for_each(node, &t_server.clients) {
                conn = list_entry(node, io_t, node);
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                // vty_out(vty, "send file data to client\r\n");
            }
            bzero(&sendbuf->file.data, BUFFER_SIZE);
            // vty_out(vty, "file data init\r\n");
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_SEND_END;
        vty_out(vty, "send file over\r\n");
        sendbuf->file.byte_size = filebytes;
        vty_out(vty, "send all bytes:%d\r\n", sendbuf->file.byte_size);
        fclose(fp);
        // vty_out(vty, "file close\r\n");

        sendbuf->len = sizeof(*sendbuf);
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            hio_write(conn->io, sendbuf, sizeof(*sendbuf));
        }
        hmutex_unlock(&s_mutex);

        HV_FREE(sendbuf);
    }
    else if (strncmp(argv[1], "group", 5) == 0) {
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (strncmp(conn->netmtclient.area, argv[2], 5) == 0) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                vty_out(vty, "\r\n******************************\r\n");
                vty_out(vty, "\rsend to %s client\r\n", argv[1]);
            }
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_DATA;
        // vty_out(vty, "send file data to client\r\n");
        int file_block_length = 0;
        //遍历整个链表
        hmutex_lock(&s_mutex);
        while ((file_block_length = fread(&sendbuf->file.data, sizeof(char), BUFFER_SIZE, fp)) > 0) {
            filebytes += file_block_length;
            sendbuf->file.data_size = file_block_length;
            // printf("read file data size:%d\n", sendbuf->file.data_size);
            // vty_out(vty, "send data to all....\r\n");
            sendbuf->len = sizeof(*sendbuf);
            list_for_each(node, &t_server.clients) {
                conn = list_entry(node, io_t, node);
                if (strncmp(conn->netmtclient.area, argv[2], 5) == 0) {
                    hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                    // vty_out(vty, "\r\n******************************\r\n");
                    // vty_out(vty, "\rsend file data to client\r\n");
                }
            }
            bzero(&sendbuf->file.data, BUFFER_SIZE);
            // vty_out(vty, "file data init\r\n");
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_SEND_END;
        vty_out(vty, "send file over\r\n");
        sendbuf->file.byte_size = filebytes;
        vty_out(vty, "send all bytes:%d\r\n", sendbuf->file.byte_size);
        fclose(fp);
        // vty_out(vty, "file close\r\n");

        sendbuf->len = sizeof(*sendbuf);
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (strncmp(conn->netmtclient.area, argv[2], 5) == 0) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
            }
        }
        hmutex_unlock(&s_mutex);

        HV_FREE(sendbuf);
    }
    else if (strncmp(argv[1], "type", 4) == 0) {
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (strncmp(conn->netmtclient.type, argv[2], 3) == 0) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                vty_out(vty, "\r\n******************************\r\n");
                vty_out(vty, "\rsend to %s client\r\n", argv[1]);
            }
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_DATA;
        // vty_out(vty, "send file data to client\r\n");
        int file_block_length = 0;
        //遍历整个链表
        hmutex_lock(&s_mutex);
        while ((file_block_length = fread(&sendbuf->file.data, sizeof(char), BUFFER_SIZE, fp)) > 0) {
            filebytes += file_block_length;
            sendbuf->file.data_size = file_block_length;
            // printf("read file data size:%d\n", sendbuf->file.data_size);
            // vty_out(vty, "send data to all....\r\n");
            sendbuf->len = sizeof(*sendbuf);
            list_for_each(node, &t_server.clients) {
                conn = list_entry(node, io_t, node);
                if (strncmp(conn->netmtclient.type, argv[2], 3) == 0) {
                    hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                    // vty_out(vty, "send file data to client\r\n");
                }
            }
            bzero(&sendbuf->file.data, BUFFER_SIZE);
            // vty_out(vty, "file data init\r\n");
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_SEND_END;
        vty_out(vty, "send file over\r\n");
        sendbuf->file.byte_size = filebytes;
        vty_out(vty, "send all bytes:%d\r\n", sendbuf->file.byte_size);
        fclose(fp);
        // vty_out(vty, "file close\r\n");

        sendbuf->len = sizeof(*sendbuf);
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (strncmp(conn->netmtclient.type, argv[2], 3) == 0) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
            }
        }
        hmutex_unlock(&s_mutex);

        HV_FREE(sendbuf);
    }
    else if (strncmp(argv[1], "single", 6) == 0) {
        int fd = atoi(argv[2]);
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (conn->io->fd == fd) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                vty_out(vty, "\r\n******************************\r\n");
                vty_out(vty, "\rsend to fd=%d client\r\n", fd);
            }
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_DATA;
        // vty_out(vty, "send file data to client\r\n");
        int file_block_length = 0;
        //遍历整个链表
        hmutex_lock(&s_mutex);
        while ((file_block_length = fread(&sendbuf->file.data, sizeof(char), BUFFER_SIZE, fp)) > 0) {
            filebytes += file_block_length;
            sendbuf->file.data_size = file_block_length;
            // printf("read file data size:%d\n", sendbuf->file.data_size);
            // vty_out(vty, "send data to all....\r\n");
            sendbuf->len = sizeof(*sendbuf);
            list_for_each(node, &t_server.clients) {
                conn = list_entry(node, io_t, node);
                if (conn->io->fd == fd) {
                    hio_write(conn->io, sendbuf, sizeof(*sendbuf));
                    // vty_out(vty, "send file data to client\r\n");
                }
            }
            bzero(&sendbuf->file.data, BUFFER_SIZE);
            // vty_out(vty, "file data init\r\n");
        }
        hmutex_unlock(&s_mutex);

        sendbuf->subcommonid = NETMT_SEND_END;
        vty_out(vty, "\rsend client %d file over\r\n", fd);
        sendbuf->file.byte_size = filebytes;
        vty_out(vty, "send all bytes:%d\r\n", sendbuf->file.byte_size);
        fclose(fp);
        // vty_out(vty, "\rsend client %d file \r\n");

        sendbuf->len = sizeof(*sendbuf);
        hmutex_lock(&s_mutex);
        list_for_each(node, &t_server.clients) {
            conn = list_entry(node, io_t, node);
            if (conn->io->fd == fd) {
                hio_write(conn->io, sendbuf, sizeof(*sendbuf));
            }
        }
        hmutex_unlock(&s_mutex);

        HV_FREE(sendbuf);
    }
    return CMD_SUCCESS;
}

DEFUN(get_netmt, get_netmt_cmd, "get FILENAME from remote CLIENT FILENAME", "get filepath from remote client(fd) filepath\n")
{
    // netmt_command_t comm;
    bzero(&comm, sizeof(comm));

    struct list_node *node;
   
    io_t *conn;
    char *localfile;
    char *remotefile;
    char *cwd;
    if (argc == 0 || argc != 3)
    {
        vty_out(vty, "invalid command.%s", VTY_NEWLINE);
        return CMD_WARNING;
    }
    //获取本地文件绝对路径
    if (!IS_DIRECTORY_SEP(*argv[0]))
    {
        cwd = getcwd(NULL, MAXPATHLEN);
        localfile = XMALLOC(MTYPE_TMP, strlen(cwd) + strlen(argv[0]) + 2);
        sprintf(localfile, "%s/%s", cwd, argv[0]);
    }
    else
    {
        localfile = argv[0];
    }
    //获取远端文件绝对路径
    if (!IS_DIRECTORY_SEP(*argv[2]))
    {
        cwd = getcwd(NULL, MAXPATHLEN);
        remotefile = XMALLOC(MTYPE_TMP, strlen(cwd) + strlen(argv[2]) + 2);
        sprintf(remotefile, "%s/%s", cwd, argv[2]);
    }
    else
    {
        remotefile = argv[2];
    }

    strncpy(comm.file.localfile, localfile, FILE_LEN);
    comm.file.localfile[FILE_LEN - 1] = '\0';
    vty_out(vty, "\r\nlocalfile:%s\r\n", comm.file.localfile);
    strncpy(comm.file.remotefile, remotefile, FILE_LEN);
    comm.file.remotefile[FILE_LEN - 1] = '\0';
    vty_out(vty, "\r\nremotefile:%s\r\n", comm.file.remotefile);
    comm.commonid = NETMT_GET;
    comm.subcommonid = NETMT_CLIENT;
    // printf("NETMT_CLIENT=%d\n", NETMT_CLIENT);

    int fd = atoi(argv[1]); //获取客户端的fd
    comm.fd = fd;
    // 找到指定的客户端
    list_for_each(node, &t_server.clients)
    {
        conn = list_entry(node, io_t, node);
        if (conn->io->fd == fd)
        {
            comm.len = sizeof(comm);
            hio_write(conn->io, &comm, comm.len);
            // printf("把信息发送给了客户端!\n");
        }
    }

    return CMD_SUCCESS;
}

DEFUN(register_netmt, register_netmt_cmd, "register", SHOW_STR "client register\n")
{
    bzero(&comm, sizeof(comm));

    struct list_node *node;
    io_t *conn;

    comm.commonid = NETMT_REGISTER;

    list_for_each(node, &t_server.clients)
    {
        conn = list_entry(node, io_t, node);
        comm.fd = conn->io->fd;
        comm.len = sizeof(comm);
        hio_write(conn->io, &comm, comm.len);
    }

    sleep(1);
    if (t_server.commonid == NETMT_REGISTER) {
        vty_out(vty, "\rclient register success\r\n");
    }else {
        vty_out(vty, "\rclient register fail\r\n");
    }
    return CMD_SUCCESS;
}

// const void* socket_set_ip(sockaddr_u* addr, char* ip, int len) {
//     if (addr->sa.sa_family == AF_INET) {
//         inet_ntop(AF_INET, ip, &addr->sin.sin_addr, len);
//     }

// }

DEFUN(ping_netmt, ping_netmt_cmd, "ping IP", SHOW_STR "server ping client ip\n")
{
    bzero(&comm, sizeof(comm));

    comm.commonid = NETMT_PING;
    if (argc == 0)
    {
        vty_out(vty, "invalid command.%s", VTY_NEWLINE);
        return CMD_WARNING;
    }

    char netmtfile[256] = "\0";

    char *inifile = "../etc/netmtServer.ini";
    if (!GetAbsPath(inifile, netmtfile, sizeof(netmtfile))) {
        vty_out(vty, "\r\nGet cfg-file failed! file='%s'\r\n", inifile);
    }

    const char serverhost[SOCKADDR_STRLEN] = {0};
    int serverport;
    // netmtaddr = NULL;
    // HV_ALLOC_SIZEOF(netmtaddr);
    if (!CGetValueString(netmtfile, "server", "host", "0.0.0.0", serverhost, "")) {
        vty_out(vty, "\rserver:host error!\r\n");
    }
    if (!CGetValueInt(netmtfile, "server", "port", 1234, &serverport, "")) {
        vty_out(vty, "\rserver:port error!\r\n");
    }

    strncpy(comm.netmtclient.client_addr.netmthost, serverhost, SOCKADDR_STRLEN);
    comm.netmtclient.client_addr.netmtport = serverport;
    comm.len = sizeof(comm);
    // for (int i = 0; i < pinglen; i++) {
    //     hio_write(udpio, &comm, sizeof(comm));
    //     sleep(10);
    // }
    hio_write(udpio, &comm, sizeof(comm));
    vty_out(vty, "\rsend ping to udp client\r\n");

    return CMD_SUCCESS;

}

DEFUN(exec_shell_remote, exec_shell_remote_cmd, "exec shell remote (all|group|single) [WORD] [WORD] [WORD] [WORD] [WORD]", SHOW_STR " exec shell remote client ifconfig\n"
                                                    "exec shell remote client(all | clientfd) tar -zxvf filepath\n")
{
    if (argc == 0)
    {
        vty_out(vty, "invalid command.%s", VTY_NEWLINE);
        vty_out(vty, "Usage tip: exec shell remote single fd ls -a.%s", VTY_NEWLINE);
        return CMD_WARNING;
    }

    bzero(&comm, sizeof(comm));
    //memset(&cmdstr_stat, 0, sizeof(cmdstr_stat));
    int i = 0;
    int ret = 0;
    int fd = 0;
    char* client = argv[0];
    comm.commonid = NETMT_EXEC;

    if ((ret = strncmp(client, "all", 3)) == 0)
    {
        for(i = 1; i < argc; i++)
        {
            strcat(comm.cmdstr, argv[i]);
            strcat(comm.cmdstr, " ");
        }
        vty_out(vty, "shellcmd : %s\r\n", comm.cmdstr);

        comm.flag = 0;

        send_total_string(&comm, &t_server);
        sleep(1);
        print_all_client_return_info(&t_server, vty);   
    } else if ((ret = strncmp(client, "group", 5)) == 0)
    {
        
    } else
    {
        fd = atoi(argv[1]);
        for(i = 2; i < argc; i++)
        {
            strcat(comm.cmdstr, argv[i]);
            strcat(comm.cmdstr, " ");
        }
        vty_out(vty, "shellcmd : %s\r\n", comm.cmdstr);
        comm.flag = 0;
        send_single_fd(fd, &comm, &t_server);
        sleep(1);
        print_all_client_return_info(&t_server, vty);
    }

    return CMD_SUCCESS;
}

//从服务端的listenio读取数据
// void vtyInf(struct vty* pvty) {
//     netmt_command_t *vtybuf = (netmt_command_t *)hevent_priority(t_server.listenio);

//     switch (vtybuf->commonid) {
//         case NETMT_PING:
//             if (vtybuf->subcommonid == NETMT_SUCCESS) {
//                 vty_out(pvty, "\rping success!\r\n");
//             }else {
//                 vty_out(pvty, "\r ping fail!\r\n");
//             }
//             break;
        
//         default:
//             break;
//     }
// }

void netmt_servercmd_init()
{
    hmutex_init(&s_mutex);
    // vtyInf(vty);

    install_node(&netmt_cmd_node, config_write_show_netmt_cmd);

    install_element(VIEW_NODE, &ping_netmt_cmd);
    install_element(VIEW_NODE, &exec_shell_remote_cmd);

    install_element(VIEW_NODE, &show_status_netmt_cmd);
    install_element(CONFIG_NODE, &show_status_netmt_cmd);
    install_element(ENABLE_NODE, &show_status_netmt_cmd);

    install_element(VIEW_NODE, &show_netmt_help_cmd);
    install_element(VIEW_NODE, &show_netmt_version_cmd);

    install_element(VIEW_NODE, &send_netmt_cmd);
    install_element(ENABLE_NODE, &send_netmt_cmd);

    install_element(VIEW_NODE, &get_netmt_cmd);
    install_element(ENABLE_NODE, &get_netmt_cmd);
    install_element(VIEW_NODE, &register_netmt_cmd);
    install_element(VIEW_NODE, &show_netmt_client_addr_cmd);

    // install_element(VIEW_NODE, &exec_shell_remote_cmd);

}
