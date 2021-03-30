#ifndef IO_COMMANDS_HPP
#define IO_COMMANDS_HPP

// forward declare CLI::App
namespace CLI {
struct App;
}

void
mkdir_init(CLI::App& app);

void
open_init(CLI::App& app);

void
opendir_init(CLI::App& app);

void
read_init(CLI::App& app);

void
pread_init(CLI::App& app);

void
readv_init(CLI::App& app);

void
preadv_init(CLI::App& app);

void
readdir_init(CLI::App& app);

void
rmdir_init(CLI::App& app);

void
stat_init(CLI::App& app);

void
write_init(CLI::App& app);

void
pwrite_init(CLI::App& app);

void
writev_init(CLI::App& app);

void
pwritev_init(CLI::App& app);

#ifdef STATX_TYPE
void
statx_init(CLI::App& app);
#endif

void
lseek_init(CLI::App& app);

void
write_validate_init(CLI::App& app);

void
write_random_init(CLI::App& app);

void
truncate_init(CLI::App& app);

// UTIL
void
file_compare_init(CLI::App& app);
void
chdir_init(CLI::App& app);

void
getcwd_validate_init(CLI::App& app);

void
symlink_init(CLI::App& app);

#endif // IO_COMMANDS_HPP
