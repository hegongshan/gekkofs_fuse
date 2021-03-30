#include <string>
#include <vector>

namespace gkfs::path {

unsigned int
match_components(const std::string& path, unsigned int& path_components,
                 const std::vector<std::string>& components);

bool
resolve(const std::string& path, std::string& resolved,
        bool resolve_last_link = true);

std::string
get_sys_cwd();

void
set_sys_cwd(const std::string& path);

void
set_env_cwd(const std::string& path);

void
unset_env_cwd();

void
init_cwd();

void
set_cwd(const std::string& path, bool internal);

} // namespace gkfs::path
