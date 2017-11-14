
#include <classes/rpc_data.hpp>

using namespace std;

// Utility functions
bool RPCData::get_addr_by_hostid(const uint64_t hostid, hg_addr_t& svr_addr) {
    return true;// XXX UNUSED
//    if (address_cache_.tryGet(hostid, svr_addr)) {
//        ADAFS_DATA->spdlogger()->debug("tryGet successful and put in svr_addr ");
//        //found
//        return true;
//    } else {
//        ADAFS_DATA->spdlogger()->debug("not found in lrucache");
//        // not found, manual lookup and add address mapping to LRU cache
//#ifndef RPC_TEST
//        auto hostname = RPC_PROTOCOL + "://"s + ADAFS_DATA->hosts().at(hostid) + ":"s +
//                        ADAFS_DATA->rpc_port(); // convert hostid to hostname and port
//#else
//        auto hostname = "bmi+tcp://127.0.0.1:" +
//                        ADAFS_DATA->rpc_port(); // convert hostid to hostname and port
////        auto hostname = "bmi+tcp://134.93.182.11:" +
////                        ADAFS_DATA->rpc_port(); // convert hostid to hostname and port
//#endif
//        ADAFS_DATA->spdlogger()->debug("generated hostid {}", hostname);
//        margo_addr_lookup(RPC_DATA->client_mid(), hostname.c_str(), &svr_addr);
//        if (svr_addr == HG_ADDR_NULL)
//            return false;
//        address_cache_.insert(hostid, svr_addr);
//        return true;
//    }
}

size_t RPCData::get_rpc_node(const std::string& to_hash) {
    return ADAFS_DATA->hashf()(to_hash) % ADAFS_DATA->host_size();
}

//std::string RPCData::get_dentry_hashable(const fuse_ino_t parent, const char* name) {
//    return fmt::FormatInt(parent).str() + "_" + name;
//}

// Getter/Setter

margo_instance* RPCData::server_rpc_mid() {
    return server_rpc_mid_;
}

void RPCData::server_rpc_mid(margo_instance* server_rpc_mid) {
    RPCData::server_rpc_mid_ = server_rpc_mid;
}

margo_instance* RPCData::server_ipc_mid() {
    return server_ipc_mid_;
}

void RPCData::server_ipc_mid(margo_instance* server_ipc_mid) {
    RPCData::server_ipc_mid_ = server_ipc_mid;
}

hg_id_t RPCData::rpc_minimal_id() const {
    return rpc_minimal_id_;
}

void RPCData::rpc_minimal_id(hg_id_t rpc_minimal_id) {
    RPCData::rpc_minimal_id_ = rpc_minimal_id;
}

lru11::Cache<uint64_t, hg_addr_t>& RPCData::address_cache() {
    return address_cache_;
}

hg_id_t RPCData::rpc_srv_create_node_id() const {
    return rpc_srv_create_node_id_;
}

void RPCData::rpc_srv_create_node_id(hg_id_t rpc_srv_create_node_id) {
    RPCData::rpc_srv_create_node_id_ = rpc_srv_create_node_id;
}

hg_id_t RPCData::rpc_srv_attr_id() const {
    return rpc_srv_attr_id_;
}

void RPCData::rpc_srv_attr_id(hg_id_t rpc_srv_attr_id) {
    RPCData::rpc_srv_attr_id_ = rpc_srv_attr_id;
}

hg_id_t RPCData::rpc_srv_read_data_id() const {
    return rpc_srv_read_data_id_;
}

void RPCData::rpc_srv_read_data_id(hg_id_t rpc_srv_read_data_id) {
    RPCData::rpc_srv_read_data_id_ = rpc_srv_read_data_id;
}

hg_id_t RPCData::rpc_srv_write_data_id() const {
    return rpc_srv_write_data_id_;
}

void RPCData::rpc_srv_write_data_id(hg_id_t rpc_srv_write_data_id) {
    RPCData::rpc_srv_write_data_id_ = rpc_srv_write_data_id;
}

hg_id_t RPCData::rpc_srv_remove_node_id() const {
    return rpc_srv_remove_node_id_;
}

void RPCData::rpc_srv_remove_node_id(hg_id_t rpc_srv_remove_node_id) {
    RPCData::rpc_srv_remove_node_id_ = rpc_srv_remove_node_id;
}








