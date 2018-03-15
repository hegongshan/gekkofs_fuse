
#include <global/rpc/rpc_utils.hpp>
#include <extern/spdlog/fmt/fmt.h>

using namespace std;

/**
 * converts std bool to mercury bool
 * @param state
 * @return
 */
hg_bool_t bool_to_merc_bool(const bool state) {
    return state ? static_cast<hg_bool_t>(HG_TRUE) : static_cast<hg_bool_t>(HG_FALSE);
}

/**
 * checks if a Mercury handle's address is shared memory
 * @param mid
 * @param addr
 * @return bool
 */
bool is_handle_sm(margo_instance_id mid, const hg_addr_t& addr) {
    hg_size_t size = 128;
    char addr_cstr[128];
    if (margo_addr_to_string(mid, addr_cstr, &size, addr) != HG_SUCCESS)
        return false;
    string addr_str(addr_cstr);
    return addr_str.substr(0, 5) == "na+sm";
}

/**
 * Determines the node id for a given path
 * @param to_hash
 * @return
 */
size_t adafs_hash_path(const string& to_hash, const size_t host_size) {
    return std::hash<string>{}(to_hash) % host_size;
}

/**
 * Determines the node id for a given path and chnk id. Wraps adafs_hash_path()
 * @param path
 * @return
 */
size_t adafs_hash_path_chunk(const string& path, const size_t chunk_id, const size_t host_size) {
    auto to_hash = path + fmt::FormatInt(chunk_id).str();
    return adafs_hash_path(to_hash, host_size);
}