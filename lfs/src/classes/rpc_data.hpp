//
// Created by evie on 6/21/17.
//

#ifndef LFS_RPC_DATA_HPP
#define LFS_RPC_DATA_HPP

#include "../main.hpp"

class RPCData {

private:
    RPCData() {}

    // Mercury Server
    hg_class_t* server_hg_class_;
    hg_context_t* server_hg_context_;

    // Mercury Client
    hg_class_t* client_hg_class_;
    hg_context_t* client_hg_context_;

    // TODO RPC IDs

public:
    static RPCData* getInstance() {
        static RPCData instance;
        return &instance;
    }

    RPCData(RPCData const&) = delete;

    void operator=(RPCData const&) = delete;

    hg_class_t* server_hg_class() const;

    void server_hg_class(hg_class_t* server_hg_class);

    hg_context_t* server_hg_context() const;

    void server_hg_context(hg_context_t* server_hg_context);

    hg_class_t* client_hg_class() const;

    void client_hg_class(hg_class_t* client_hg_class);

    hg_context_t* client_hg_context() const;

    void client_hg_context(hg_context_t* client_hg_context);

};


#endif //LFS_RPC_DATA_HPP
