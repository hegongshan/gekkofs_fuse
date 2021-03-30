#include <global/env_util.hpp>

#include <string>
#include <cstdlib>

using namespace std;

string
gkfs::env::get_var(const string& name, const string& default_value) {
    const char* const val = ::secure_getenv(name.c_str());
    return val != nullptr ? string(val) : default_value;
}
