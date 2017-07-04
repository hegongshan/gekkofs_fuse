//
// Created by evie on 6/21/17.
//

#include "rpc_data.hpp"


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

lru11::Cache<std::string, hg_addr_t>& RPCData::address_cache() {
    return address_cache_;
}

