#include <global/global_func.hpp>

using namespace std;

string daemon_pid_path() {
    return (DAEMON_AUX_PATH + "/adafs_daemon.pid"s);
}
