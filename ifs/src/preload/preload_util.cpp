
#include <preload/preload_util.hpp>

#include <dirent.h>
#include <fstream>

using namespace std;

static const std::string dentry_val_delim = ","s;

bool is_fs_path(const char* path) {
    return strstr(path, fs_config->mountdir.c_str()) == path;
}

// TODO merge the two stat functions
/**
 * Converts the dentry db value into a stat struct, which is needed by Linux
 * @param path
 * @param db_val
 * @param attr
 * @return
 */
int db_val_to_stat(const std::string path, std::string db_val, struct stat& attr) {

    auto pos = db_val.find(dentry_val_delim);
    if (pos == std::string::npos) { // no delimiter found => no metadata enabled. fill with dummy values
        attr.st_ino = std::hash<std::string>{}(path);
        attr.st_mode = static_cast<unsigned int>(stoul(db_val));
        attr.st_nlink = 1;
        attr.st_uid = fs_config->uid;
        attr.st_gid = fs_config->gid;
        attr.st_size = 0;
        attr.st_blksize = BLOCKSIZE;
        attr.st_blocks = 0;
        attr.st_atim.tv_sec = 0;
        attr.st_mtim.tv_sec = 0;
        attr.st_ctim.tv_sec = 0;
        return 0;
    }
    // some metadata is enabled: mode is always there
    attr.st_mode = static_cast<unsigned int>(stoul(db_val.substr(0, pos)));
    db_val.erase(0, pos + 1);
    // size is also there XXX
    pos = db_val.find(dentry_val_delim);
    if (pos != std::string::npos) {  // delimiter found. more metadata is coming
        attr.st_size = stol(db_val.substr(0, pos));
        db_val.erase(0, pos + 1);
    } else {
        attr.st_size = stol(db_val);
        return 0;
    }
    // The order is important. don't change.
    if (fs_config->atime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_atim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->mtime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_mtim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->ctime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_ctim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->uid_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_uid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->gid_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_gid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->inode_no_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_ino = static_cast<ino_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->link_cnt_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_nlink = static_cast<nlink_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->blocks_state) { // last one will not encounter a delimiter anymore
        attr.st_blocks = static_cast<blkcnt_t>(stoul(db_val));
    }
    return 0;
}

/**
 * Converts the dentry db value into a stat64 struct, which is needed by Linux
 * @param path
 * @param db_val
 * @param attr
 * @return
 */
int db_val_to_stat64(const std::string path, std::string db_val, struct stat64& attr) {

    auto pos = db_val.find(dentry_val_delim);
    if (pos == std::string::npos) { // no delimiter found => no metadata enabled. fill with dummy values
        attr.st_ino = std::hash<std::string>{}(path);
        attr.st_mode = static_cast<unsigned int>(stoul(db_val));
        attr.st_nlink = 1;
        attr.st_uid = fs_config->uid;
        attr.st_gid = fs_config->gid;
        attr.st_size = 0;
        attr.st_blksize = BLOCKSIZE;
        attr.st_blocks = 0;
        attr.st_atim.tv_sec = 0;
        attr.st_mtim.tv_sec = 0;
        attr.st_ctim.tv_sec = 0;
        return 0;
    }
    // some metadata is enabled: mode is always there
    attr.st_mode = static_cast<unsigned int>(stoul(db_val.substr(0, pos)));
    db_val.erase(0, pos + 1);
    // size is also there XXX
    pos = db_val.find(dentry_val_delim);
    if (pos != std::string::npos) {  // delimiter found. more metadata is coming
        attr.st_size = stol(db_val.substr(0, pos));
        db_val.erase(0, pos + 1);
    } else {
        attr.st_size = stol(db_val);
        return 0;
    }
    // The order is important. don't change.
    if (fs_config->atime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_atim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->mtime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_mtim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->ctime_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_ctim.tv_sec = static_cast<time_t>(stol(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->uid_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_uid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->gid_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_gid = static_cast<uid_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->inode_no_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_ino = static_cast<ino_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->link_cnt_state) {
        pos = db_val.find(dentry_val_delim);
        attr.st_nlink = static_cast<nlink_t>(stoul(db_val.substr(0, pos)));
        db_val.erase(0, pos + 1);
    }
    if (fs_config->blocks_state) { // last one will not encounter a delimiter anymore
        attr.st_blocks = static_cast<blkcnt_t>(stoul(db_val));
    }
    return 0;
}

/**
 * Returns the process id for a process name
 * @param procName
 * @return
 */
int getProcIdByName(string procName) {
    int pid = -1;

    // Open the /proc directory
    DIR* dp = opendir("/proc");
    if (dp != nullptr) {
        // Enumerate all entries in directory until process found
        struct dirent* dirp;
        while (pid < 0 && (dirp = readdir(dp))) {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0) {
                // Read contents of virtual /proc/{pid}/cmdline file
                string cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
                ifstream cmdFile(cmdPath.c_str());
                string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty()) {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (procName == cmdLine)
                        pid = id;
                }
            }
        }
    }

    closedir(dp);

    return pid;
}

/**
 * Returns the path where daemon process writes information for the running clients
 * @return string
 */
string daemon_register_path(int pid) {
    return (DAEMON_AUX_PATH + "/daemon_"s + to_string(pid) + ".run"s);
}

bool get_daemon_auxiliaries() {
    auto ret = false;
    auto adafs_daemon_pid = getProcIdByName("adafs_daemon"s);
    if (adafs_daemon_pid == -1) {
        ld_logger->error("{}() ADA-FS daemon not started. Exiting ...", __func__);
        perror("ADA-FS daemon not started. Exiting ...");
        return false;
    }
    ifstream ifs(daemon_register_path(adafs_daemon_pid).c_str(), ::ifstream::in);
    string mountdir;
    if (ifs) {
        getline(ifs, mountdir);
        if (!mountdir.empty()) {
            ret = true;
            fs_config->mountdir = mountdir;
        } else {
            ld_logger->error("{}() Error reading daemon auxiliary file", __func__);
        }
    }
    if (ifs.bad()) {
        perror("Error opening file to register daemon process");
        ld_logger->error("{}() Error opening file to daemon auxiliary file", __func__);
        return false;
    }
    ifs.close();

    return ret;
}

/**
 * Creates an abstract rpc address for a given hostid and puts it into an address cache map
 * @param hostid
 * @param svr_addr
 * @return
 */
bool get_addr_by_hostid(const uint64_t hostid, hg_addr_t& svr_addr) {

    if (rpc_address_cache.tryGet(hostid, svr_addr)) {
        ld_logger->trace("tryGet successful and put in svr_addr");
        //found
        return true;
    } else {
        ld_logger->trace("not found in lrucache");
        // not found, manual lookup and add address mapping to LRU cache
        auto hostname = RPC_PROTOCOL + "://"s + fs_config->hosts.at(hostid) + HOSTNAME_SUFFIX + ":"s +
                        fs_config->rpc_port; // convert hostid to hostname and port
        ld_logger->trace("generated hostname {} with rpc_port {}", hostname, fs_config->rpc_port);
        // try to look up 3 times before erroring out
        hg_return_t ret;
        // TODO If this is solution is somewhat helpful, write a more versatile solution
        for (unsigned int i = 0; i < 3; i++) {
            ret = margo_addr_lookup(ld_margo_rpc_id, hostname.c_str(), &svr_addr);
            if (ret != HG_SUCCESS) {
                // still not working after 5 tries.
                if (i == 4) {
                    ld_logger->error("{}() Unable to lookup address {} from host {}", __func__,
                                     hostname, fs_config->hosts.at(fs_config->host_id));
                    return false;
                }
                // Wait a second then try again
                sleep(1 * (i + 1));
            } else {
                break;
            }
        }
        if (svr_addr == HG_ADDR_NULL) {
            ld_logger->error("{}() looked up address is NULL for address {} from host {}", __func__,
                             hostname, fs_config->hosts.at(fs_config->host_id));
            return false;
        }
        rpc_address_cache.insert(hostid, svr_addr);
        return true;
    }
}

/**
 * Determines the node id for a given path
 * @param to_hash
 * @return
 */
size_t get_rpc_node(const string& to_hash) {
    return std::hash<string>{}(to_hash) % fs_config->host_size;
}

/**
 * Determines if the recipient id in an RPC is refering to the local or an remote node
 * @param recipient
 * @return
 */
bool is_local_op(const size_t recipient) {
    return recipient == fs_config->host_id;
}

inline hg_return
margo_create_wrap_helper(const hg_id_t ipc_id, const hg_id_t rpc_id, const size_t recipient, hg_handle_t& handle,
                         hg_addr_t& svr_addr, bool force_rpc) {
    hg_return_t ret;
    if (is_local_op(recipient) && !force_rpc) { // local
        ret = margo_create(ld_margo_ipc_id, daemon_svr_addr, ipc_id, &handle);
        ld_logger->debug("{}() to local daemon (IPC)", __func__);
    } else { // remote
        // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
        if (!get_addr_by_hostid(recipient, svr_addr)) {
            ld_logger->error("{}() server address not resolvable for host id {}", __func__, recipient);
            return HG_OTHER_ERROR;
        }
        ret = margo_create(ld_margo_rpc_id, svr_addr, rpc_id, &handle);
        ld_logger->debug("{}() to remote daemon (RPC)", __func__);
    }
    if (ret != HG_SUCCESS) {
        ld_logger->error("{}() creating handle FAILED", __func__);
        return HG_OTHER_ERROR;
    }
    return ret;
}

/**
 * Wraps certain margo functions to create a Mercury handle
 * @param ipc_id
 * @param rpc_id
 * @param path
 * @param handle
 * @param svr_addr
 * @return
 */
template<>
hg_return margo_create_wrap(const hg_id_t ipc_id, const hg_id_t rpc_id, const std::string& path, hg_handle_t& handle,
                            hg_addr_t& svr_addr, bool force_rpc) {
    return margo_create_wrap_helper(ipc_id, rpc_id, get_rpc_node(path), handle, svr_addr, force_rpc);
}

/**
 * Wraps certain margo functions to create a Mercury handle
 * @param ipc_id
 * @param rpc_id
 * @param recipient
 * @param handle
 * @param svr_addr
 * @return
 */
template<>
hg_return margo_create_wrap(const hg_id_t ipc_id, const hg_id_t rpc_id, const size_t& recipient, hg_handle_t& handle,
                            hg_addr_t& svr_addr, bool force_rpc) {
    return margo_create_wrap_helper(ipc_id, rpc_id, recipient, handle, svr_addr, force_rpc);
}