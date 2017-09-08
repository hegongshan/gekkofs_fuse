//
// Created by evie on 6/21/17.
//

#include <classes/rpc_data.hpp>


// Utility functions

bool RPCData::get_addr_by_hostid(const uint64_t hostid, hg_addr_t& svr_addr) {

    if (address_cache_.tryGet(hostid, svr_addr)) {
        ADAFS_DATA->spdlogger()->debug("tryGet successful and put in svr_addr ");
        //found
        return true;
    } else {
        ADAFS_DATA->spdlogger()->debug("not found in lrucache");
        // not found, manual lookup and add address mapping to LRU cache
#ifndef RPC_TEST
        auto hostname = "cci+tcp://" + ADAFS_DATA->hosts().at(hostid) + ":" +
                        ADAFS_DATA->rpc_port(); // convert hostid to hostname and port
#else
        auto hostname = "cci+tcp://127.0.0.1:" +
                        ADAFS_DATA->rpc_port(); // convert hostid to hostname and port
//        auto hostname = "cci+tcp://134.93.182.11:" +
//                        ADAFS_DATA->rpc_port(); // convert hostid to hostname and port
#endif
        ADAFS_DATA->spdlogger()->debug("generated hostid {}", hostname);
        margo_addr_lookup(RPC_DATA->client_mid(), hostname.c_str(), &svr_addr);
        if (svr_addr == HG_ADDR_NULL)
            return false;
        address_cache_.insert(hostid, svr_addr);
        return true;
    }
}

size_t RPCData::get_rpc_node(const std::string& to_hash) {
    return ADAFS_DATA->hashf()(to_hash) % ADAFS_DATA->host_size();
}

//std::string RPCData::get_dentry_hashable(const fuse_ino_t parent, const char* name) {
//    return fmt::FormatInt(parent).str() + "_" + name;
//}

// Getter/Setter

hg_class_t* RPCData::server_hg_class() const {
    return server_hg_class_;
}

void RPCData::server_hg_class(hg_class_t* server_hg_class) {
    RPCData::server_hg_class_ = server_hg_class;
}

hg_context_t* RPCData::server_hg_context() const {
    return server_hg_context_;
}

void RPCData::server_hg_context(hg_context_t* server_hg_context) {
    RPCData::server_hg_context_ = server_hg_context;
}

hg_class_t* RPCData::client_hg_class() const {
    return client_hg_class_;
}

void RPCData::client_hg_class(hg_class_t* client_hg_class) {
    RPCData::client_hg_class_ = client_hg_class;
}

hg_context_t* RPCData::client_hg_context() const {
    return client_hg_context_;
}

void RPCData::client_hg_context(hg_context_t* client_hg_context) {
    RPCData::client_hg_context_ = client_hg_context;
}

margo_instance* RPCData::server_mid() {
    return server_mid_;
}

void RPCData::server_mid(margo_instance* server_mid) {
    RPCData::server_mid_ = server_mid;
}

margo_instance* RPCData::client_mid() {
    return client_mid_;
}

void RPCData::client_mid(margo_instance* client_mid) {
    RPCData::client_mid_ = client_mid;
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








