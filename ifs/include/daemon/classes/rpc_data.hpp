
#ifndef LFS_RPC_DATA_HPP
#define LFS_RPC_DATA_HPP

#include "../../../main.hpp"
#include <extern/lrucache/LRUCache11.hpp>

class RPCData {

private:
    RPCData() {}

    // Margo IDs. They can also be used to retrieve the Mercury classes and contexts that were created at init time
    margo_instance_id server_rpc_mid_;
    margo_instance_id server_ipc_mid_;

    lru11::Cache<uint64_t, hg_addr_t> address_cache_{32768, 4096}; // XXX Set values are not based on anything...

    // TODO RPC client IDs
    // RPC client IDs
    hg_id_t rpc_minimal_id_;
    hg_id_t rpc_srv_create_node_id_;
    hg_id_t rpc_srv_attr_id_;
    hg_id_t rpc_srv_remove_node_id_;
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

    size_t get_rpc_node(const std::string& to_hash);

//    std::string get_dentry_hashable(const fuse_ino_t parent, const char* name);

    // Getter/Setter

    margo_instance* server_rpc_mid();

    void server_rpc_mid(margo_instance* server_rpc_mid);

    margo_instance* server_ipc_mid();

    void server_ipc_mid(margo_instance* server_ipc_mid);

    hg_id_t rpc_minimal_id() const;

    void rpc_minimal_id(hg_id_t rpc_minimal_id);

    lru11::Cache<uint64_t, hg_addr_t>& address_cache();

    hg_id_t rpc_srv_create_node_id() const;

    void rpc_srv_create_node_id(hg_id_t rpc_srv_create_node_id);

    hg_id_t rpc_srv_attr_id() const;

    void rpc_srv_attr_id(hg_id_t rpc_srv_attr_id);

    hg_id_t rpc_srv_read_data_id() const;

    hg_id_t rpc_srv_remove_node_id() const;

    void rpc_srv_remove_node_id(hg_id_t rpc_srv_remove_node_id);

    void rpc_srv_read_data_id(hg_id_t rpc_srv_read_data_id);

    hg_id_t rpc_srv_write_data_id() const;

    void rpc_srv_write_data_id(hg_id_t rpc_srv_write_data_id);

};


#endif //LFS_RPC_DATA_HPP
