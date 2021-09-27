#ifndef __NETMT_H__
#define __NETMT_H__

#include "hloop.h"
#include "hsocket.h"
#include "event/hevent.h"
#include "command.h"
#include "vty.h"


// arg_type
#define NO_ARGUMENT         0
#define REQUIRED_ARGUMENT   1
#define OPTIONAL_ARGUMENT   2
// option define
#define OPTION_PREFIX   '-'
#define OPTION_DELIM    '='
#define OPTION_ENABLE   "on"
#define OPTION_DISABLE  "off"
#define MAX_OPTION      32
// opt type
#define NOPREFIX_OPTION 0
#define SHORT_OPTION    -1
#define LONG_OPTION     -2

#define FILE_LEN 256
#define LEN 64
#define BUFFER_SIZE 1024
#define POPEN_SIZE 4096
#define DATABUF 20480  //2M  
#define HeartTime 240000 //ms  

/* Common descriptions. */
#define SEND_SUCCESS "send file success\n"
#define GET_SUCCESS "get file success\n"

//定义创建文件时的模式，此处对用户，组，其他设置的权限都是可读可写。
#define MODE_RW_UGO (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

typedef struct option_s {
    char        short_opt;
    const char* long_opt;
    int         arg_type;
} option_t;

typedef struct addr_s {
    char netmthost[SOCKADDR_STRLEN];
    int netmtport;
} addr_t;

typedef struct netmtserver_t {
    addr_t client_addr;
    int id;
    char area[LEN];
    char type[LEN];
} netmtclient_s;

//每个命令对应
typedef enum {
    NETMT_HELP = 1,
    NETMT_CLIENT,
    NETMT_SEND,
    NETMT_GET,
    NETMT_REGISTER,
    NETMT_PING,
    NETMT_COMMAND,
    NETMT_HEARTBEAT,
    NETMT_EXEC
} netmt_command_e;

typedef enum {
    NETMT_SEND_END = 1,
    NETMT_SEND_SUCCESS,
    NETMT_GET_END,
    NETMT_GET_SUCCESS,
    NETMT_FILE_OPEN,
    NETMT_DATA,
    NETMT_SUCCESS,
    NETMT_EXEC_SHELL,
    NETMT_EXEC_SHELL_RESULT,
    NETMT_EXEC_SHELL_ENDFLAG,
    NETMT_EXEC_SHELL_SUCCESS
} netmt_subcommand_e;

typedef struct netmt_file_s {
    char localfile[FILE_LEN];
    char remotefile[FILE_LEN];
    char data[BUFFER_SIZE];
    int byte_size;  //文件大小
    int data_size;
} netmt_file_t;

typedef struct netmt_command_s {
    int len; //
    int version;
    u_int commonid;
    u_int subcommonid;
    int c_num;  //统计发给客户端的个数
    int fd;
    int id; //用于标识客户端
    int flag;
    uint64_t last_send_ping_time;
    uint64_t last_recv_pong_time;
    char cmdstr[BUFFER_SIZE];
    union {
        netmt_file_t file;
        netmtclient_s netmtclient; //存储客户端信息
        char buf[LEN];
        char total_result[POPEN_SIZE];
    };
} netmt_command_t;

//服务器主io
typedef struct server_s {
    hloop_t* loop;
    hio_t*   listenio;
    int commonid;
    int id;
    int flag;
    struct list_node clients;
} server_t;
//客户端的结构
typedef struct io_s {
    hio_t* io;
    addr_t netmtaddr;
    netmtclient_s netmtclient;
    struct list_node node;

    uint64_t last_send_ping_time;
    uint64_t last_recv_pong_time;
    int count;
    char name[64];//客户端名字
    int flag;//1表示数据接收完成，0表示未完成
    char success[FILE_LEN];
    //存储方案1 ：采用固定缓冲区
    char buffer[DATABUF];
    int buffer_max_size;
    int buffer_current_size;
} io_t;

typedef struct conf_ctx_s {
    // IniParser* parser;
    int loglevel;
    int worker_processes;
    int worker_threads;
    int port;
} conf_ctx_t;

typedef struct main_ctx_s {
    char    run_dir[MAX_PATH];
    char    program_name[MAX_PATH];

    char    confile[MAX_PATH]; // default etc/${program}.conf
    char    pidfile[MAX_PATH]; // default logs/${program}.pid
    char    logfile[MAX_PATH]; // default logs/${program}.log

    pid_t   pid;    // getpid
    pid_t   oldpid; // getpid_from_pidfile

    // arg
    int     argc;
    int     arg_len;
    char**  os_argv;
    char**  save_argv;
    char*   cmdline;
    // keyval_t    arg_kv;
    // StringList  arg_list;

    // env
    int     envc;
    int     env_len;
    char**  os_envp;
    char**  save_envp;
    // keyval_t    env_kv;

    // signals
    procedure_t reload_fn;
    void*       reload_userdata;
    // master workers model
    int             worker_processes;
    int             worker_threads;
    procedure_t     worker_fn;
    void*           worker_userdata;
    // proc_ctx_t*     proc_ctxs;
} main_ctx_t;

#endif
