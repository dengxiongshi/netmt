//
// Created by dengxs on 2021/7/7 10:28.
//
#include <stdbool.h>
#include <stdio.h>

#include "icmp.h"
#include "hlog.h"
#include "hfile.h"
#include "hmain.h"
#include "hbase.h"

// #define FATAL "../out/log/fatal.txt"

int main_ctx_init(int argc, char** argv) {
    if (argc == 0 || argv == NULL) {
        argc = 1;
        SAFE_ALLOC(argv, 2 * sizeof(char*));
        SAFE_ALLOC(argv[0], MAX_PATH);
        get_executable_path(argv[0], MAX_PATH);
    }

    get_run_dir(g_main_ctx.run_dir, sizeof(g_main_ctx.run_dir));
    //printf("run_dir=%s\n", g_main_ctx.run_dir);
    strncpy(g_main_ctx.program_name, hv_basename(argv[0]), sizeof(g_main_ctx.program_name));
#ifdef OS_WIN
    if (strcmp(g_main_ctx.program_name+strlen(g_main_ctx.program_name)-4, ".exe") == 0) {
        *(g_main_ctx.program_name+strlen(g_main_ctx.program_name)-4) = '\0';
    }
#endif
    //printf("program_name=%s\n", g_main_ctx.program_name);
    char logdir[MAX_PATH] = {0};
    snprintf(logdir, sizeof(logdir), "%s/logs/%s.log", g_main_ctx.run_dir);
    hv_mkdir(logdir);
    snprintf(g_main_ctx.confile, sizeof(g_main_ctx.confile), "%s/etc/%s.conf", g_main_ctx.run_dir, g_main_ctx.program_name);
    snprintf(g_main_ctx.pidfile, sizeof(g_main_ctx.pidfile), "%s/logs/%s.pid", g_main_ctx.run_dir, g_main_ctx.program_name);
    snprintf(g_main_ctx.logfile, sizeof(g_main_ctx.confile), "%s/logs/%s.log", g_main_ctx.run_dir, g_main_ctx.program_name);
    hlog_set_file(g_main_ctx.logfile);

    g_main_ctx.pid = getpid();
    g_main_ctx.oldpid = getpid_from_pidfile();
#ifdef OS_UNIX
    if (kill(g_main_ctx.oldpid, 0) == -1 && errno == ESRCH) {
        g_main_ctx.oldpid = -1;
    }
#else
    HANDLE hproc = OpenProcess(PROCESS_TERMINATE, FALSE, g_main_ctx.oldpid);
    if (hproc == NULL) {
        g_main_ctx.oldpid = -1;
    }
    else {
        CloseHandle(hproc);
    }
#endif

    // save arg
    int i = 0;
    g_main_ctx.os_argv = argv;
    g_main_ctx.argc = 0;
    g_main_ctx.arg_len = 0;
    for (i = 0; argv[i]; ++i) {
        g_main_ctx.arg_len += strlen(argv[i]) + 1;
    }
    g_main_ctx.argc = i;
    char* argp = NULL;
    SAFE_ALLOC(argp, g_main_ctx.arg_len);
    SAFE_ALLOC(g_main_ctx.save_argv, (g_main_ctx.argc + 1) * sizeof(char*));
    char* cmdline = NULL;
    SAFE_ALLOC(cmdline, g_main_ctx.arg_len);
    g_main_ctx.cmdline = cmdline;
    for (i = 0; argv[i]; ++i) {
        g_main_ctx.save_argv[i] = argp;
        strcpy(g_main_ctx.save_argv[i], argv[i]);
        argp += strlen(argv[i]) + 1;

        strcpy(cmdline, argv[i]);
        cmdline += strlen(argv[i]);
        *cmdline = ' ';
        ++cmdline;
    }
    g_main_ctx.save_argv[g_main_ctx.argc] = NULL;
    g_main_ctx.cmdline[g_main_ctx.arg_len-1] = '\0';

#if defined(OS_WIN) || defined(OS_LINUX) || defined(OS_DARWIN)
    // save env
    g_main_ctx.os_envp = environ;
    g_main_ctx.envc = 0;
    g_main_ctx.env_len = 0;
    for (i = 0; environ[i]; ++i) {
        g_main_ctx.env_len += strlen(environ[i]) + 1;
    }
    g_main_ctx.envc = i;
    char* envp = NULL;
    SAFE_ALLOC(envp, g_main_ctx.env_len);
    SAFE_ALLOC(g_main_ctx.save_envp, (g_main_ctx.envc + 1) * sizeof(char*));
    for (i = 0; environ[i]; ++i) {
        g_main_ctx.save_envp[i] = envp;
        strcpy(g_main_ctx.save_envp[i], environ[i]);
        envp += strlen(environ[i]) + 1;
    }
    g_main_ctx.save_envp[g_main_ctx.envc] = NULL;

    // parse env
    for (i = 0; environ[i]; ++i) {
        char* b = environ[i];
        char* delim = strchr(b, '=');
        if (delim == NULL) {
            continue;
        }
        g_main_ctx.env_kv[std::string(b, delim-b)] = std::string(delim+1);
    }
#endif

    // signals
    g_main_ctx.reload_fn = NULL;
    g_main_ctx.reload_userdata = NULL;

    // master workers
    g_main_ctx.worker_processes = 0;
    g_main_ctx.worker_threads = 0;
    g_main_ctx.worker_fn = 0;
    g_main_ctx.worker_userdata = 0;
    g_main_ctx.proc_ctxs = NULL;

    return 0;
}


bool _Ping(int argc, char** argv, const char *host, int ping_cnt) {
    // logger_t *logger;
    // // const char *filepath = FATAL;
    // // HFile file;
    // logger = logger_create();
    main_ctx_t g_main_ctx;

    // int ok_cnt = ping(host, ping_cnt);
    int ok_cnt = -1;
    if (ok_cnt < 0) {
        printf("ping fatal!");
        // if (file.open(filepath.c_str(), "wb") != 0) {
        //     fprintf(stderr, "Failed to open file %s\n", filepath.c_str());
        //     return -20;
        // }
        // //获取运行程序的路径
        // get_run_dir(g_main_ctx.run_dir, sizeof(g_main_ctx.run_dir));
        // strncpy(g_main_ctx.program_name, hv_basename("/../out/bin/"), sizeof(g_main_ctx.program_name));
        // char logdir[MAX_PATH] = {0};
        // //日志存放路径
        // snprintf(logdir, sizeof(logdir), "%s/../out/logs", g_main_ctx.run_dir);
        // //创建日志目录
        // hv_mkdir(logdir);
        // snprintf(g_main_ctx.logfile, sizeof(g_main_ctx.logfile), "%s/../out/logs/%s.log", g_main_ctx.run_dir, g_main_ctx.program_name);
        // hlog_set_level_by_str("FATAL");
        // hlog_set_file(g_main_ctx.logfile);
        main_ctx_init(argc, argv);
        // hlog_set_file(filepath);
        hlogf("Ping Fatal");
        return false;
    }else {
        // printf("ping %d count, %d ok.\n", ping_cnt, ok_cnt);
        printf("PONG\n");
        return true;
    }
}


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