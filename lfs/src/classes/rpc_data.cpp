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
