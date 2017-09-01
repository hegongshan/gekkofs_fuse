//
// Created by evie on 6/21/17.
//

#ifndef LFS_RPC_DATA_HPP
#define LFS_RPC_DATA_HPP

#include "../../main.hpp"
#include "extern/lrucache/LRUCache11.hpp"

class RPCData {

private:
    RPCData() {}

    // Can't use shared pointers here 'cause the Mercury environment has problems with it, e.g., unable to finalize,
    // resulting into a faulty fuse shutdown
    // Mercury Server
    hg_class_t* server_hg_class_;
    hg_context_t* server_hg_context_;

    // Mercury Client
    hg_class_t* client_hg_class_;
    hg_context_t* client_hg_context_;

    // Margo IDs. They can also be used to retrieve the Mercury classes and contexts that were created at init time
    margo_instance_id server_mid_;
    margo_instance_id client_mid_;

    lru11::Cache<uint64_t, hg_addr_t> address_cache_{32768, 4096}; // XXX Set values are not based on anything...

    // TODO RPC client IDs
    // RPC client IDs
    hg_id_t rpc_minimal_id_;
    hg_id_t rpc_srv_create_dentry_id_;
    hg_id_t rpc_srv_create_mdata_id_;
    hg_id_t rpc_srv_attr_id_;
    hg_id_t rpc_srv_lookup_id_;
    hg_id_t rpc_srv_remove_dentry_id_;
    hg_id_t rpc_srv_remove_mdata_id_;
    hg_id_t rpc_srv_read_data_id_;
    hg_id_t rpc_srv_write_data_id_;


public:
    static RPCData* getInstance() {
        static RPCData instance;
        return &instance;
    }

    hg_addr_t svr_addr_ = HG_ADDR_NULL; // XXX TEMPORARY! server addresses will be put into a LRU map

    RPCData(RPCData const&) = delete;

    void operator=(RPCData const&) = delete;

    // Utility functions

    bool get_addr_by_hostid(const uint64_t hostid, hg_addr_t& svr_addr);

    size_t get_rpc_node(std::string to_hash);

//    std::string get_dentry_hashable(const fuse_ino_t parent, const char* name);

    // Getter/Setter

    hg_class_t* server_hg_class() const;

    void server_hg_class(hg_class_t* server_hg_class);

    hg_context_t* server_hg_context() const;

    void server_hg_context(hg_context_t* server_hg_context);

    hg_class_t* client_hg_class() const;

    void client_hg_class(hg_class_t* client_hg_class);

    hg_context_t* client_hg_context() const;

    void client_hg_context(hg_context_t* client_hg_context);

    margo_instance* server_mid();

    void server_mid(margo_instance* server_mid);

    margo_instance* client_mid();

    void client_mid(margo_instance* client_mid);

    hg_id_t rpc_minimal_id() const;

    void rpc_minimal_id(hg_id_t rpc_minimal_id);

    lru11::Cache<uint64_t, hg_addr_t>& address_cache();

    hg_id_t rpc_srv_attr_id() const;

    void rpc_srv_attr_id(hg_id_t rpc_srv_attr_id);

    hg_id_t rpc_srv_create_dentry_id() const;

    void rpc_srv_create_dentry_id(hg_id_t rpc_srv_create_dentry_id);

    hg_id_t rpc_srv_create_mdata_id() const;

    void rpc_srv_create_mdata_id(hg_id_t rpc_srv_create_mdata_id);

    hg_id_t rpc_srv_lookup_id() const;

    void rpc_srv_lookup_id(hg_id_t rpc_srv_lookup_id);

    hg_id_t rpc_srv_remove_dentry_id() const;

    void rpc_srv_remove_dentry_id(hg_id_t rpc_srv_remove_dentry_id);

    hg_id_t rpc_srv_remove_mdata_id() const;

    void rpc_srv_remove_mdata_id(hg_id_t rpc_srv_remove_mdata_id);

    hg_id_t rpc_srv_read_data_id() const;

    void rpc_srv_read_data_id(hg_id_t rpc_srv_read_data_id);

    hg_id_t rpc_srv_write_data_id() const;

    void rpc_srv_write_data_id(hg_id_t rpc_srv_write_data_id);
};


#endif //LFS_RPC_DATA_HPP
