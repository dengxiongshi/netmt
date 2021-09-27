#include "order.h"

// int main(int argc, char** argv) {
//     order_main(argc, argv);
//     return 0;
// }
int main(int argc, char** argv) {
    main_ctx_init(argc, argv);

    if (argc == 1) {
        print_help();
        exit(10);
    }
    int ret = parse_opt_long(argc, argv, long_options, ARRAY_SIZE(long_options));
    if (ret != 0) {
        print_help();
        exit(ret);
    }

    if (get_arg("h")) {
        print_help();
        exit(0);
    }

    // version
    if (get_arg("v")) {
        print_version();
        exit(0);
    }

    // g_conf_ctx
    conf_ctx_init(&g_conf_ctx);
    const char* confile = get_arg("c");
    if (confile) {
        strncpy(g_main_ctx.confile, confile, sizeof(g_main_ctx.confile));
    }
    parse_confile(g_main_ctx.confile);

    // test
    if (get_arg("t")) {
        printf("Test confile [%s] OK!\n", g_main_ctx.confile);
        exit(0);
    }

    // signal
    signal_init(on_reload);
    const char* signal = get_arg("s");
    if (signal) {
        signal_handle(signal);
    }

#ifdef OS_UNIX
    // daemon
    if (get_arg("d")) {
        // nochdir, noclose
        int ret = daemon(1, 1);
        if (ret != 0) {
            printf("daemon error: %d\n", ret);
            exit(-10);
        }
    }
#endif

    //command,增加命令行功能
    if (get_arg("o")) {
        if (argc < 3) {
            printf("%s\n, usage:%s", "invalid command", command);
        }else if (argc == 3) {
            ls_dir(argc, argv);
        }else {
            if (argv[2] == "ls") {
                ls_dir(argc, argv);
            }
            if (argv[2] == "mkdir") {
                hv_mkdir_p(argv[3]);
            }
            if (argv[2] == "rmdir") {
                hv_rmdir_p(argv[3]);
            }
            if (argv[2] == "touch") {
                mytouch(argv[3]);
            }
            if (argv[2] == "cat") {
                my_cat(argv[3]);
            }
            if (argv[2] == "vim") {
                my_vim(argv[3]);
            }
        }
    }

    // pidfile
    create_pidfile();

    master_workers_run(worker_fn, (void*)(intptr_t)g_conf_ctx.port, g_conf_ctx.worker_processes, g_conf_ctx.worker_threads);

    return 0;
}