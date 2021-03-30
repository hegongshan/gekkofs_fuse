//#include <io/command.hpp>

#include <cstdlib>
#include <string>
#include <CLI/CLI.hpp>
#include <commands.hpp>

void
init_commands(CLI::App& app) {
    open_init(app);
    opendir_init(app);
    mkdir_init(app);
    read_init(app);
    pread_init(app);
    readv_init(app);
    preadv_init(app);
    readdir_init(app);
    rmdir_init(app);
    stat_init(app);
    write_init(app);
    pwrite_init(app);
    writev_init(app);
    pwritev_init(app);
#ifdef STATX_TYPE
    statx_init(app);
#endif
    lseek_init(app);
    write_validate_init(app);
    write_random_init(app);
    truncate_init(app);
    // utils
    file_compare_init(app);
    chdir_init(app);
    getcwd_validate_init(app);
    symlink_init(app);
}


int
main(int argc, char* argv[]) {

    CLI::App app{"GekkoFS I/O client"};
    app.require_subcommand(1);
    app.get_formatter()->label("REQUIRED", "");
    app.set_help_all_flag("--help-all", "Expand all help");
    init_commands(app);
    CLI11_PARSE(app, argc, argv);

    return EXIT_SUCCESS;
}
