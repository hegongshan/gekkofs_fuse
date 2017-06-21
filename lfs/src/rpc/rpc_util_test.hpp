//
// Created by evie on 6/14/17.
//

#ifndef LFS_INIT_HPP
#define LFS_INIT_HPP

#include "../main.hpp"

struct run_my_rpc_args {
    int val;
    margo_instance_id mid;
    hg_context_t* hg_context;
    hg_class_t* hg_class;
    hg_addr_t svr_addr;
};


bool init_margo_server_test();

void register_server_rpcs_test(hg_class_t* hg_class);

bool init_margo_client_test();

void run_my_rpc(margo_instance_id& margo_id, hg_class_t* hg_class, hg_context_t* hg_context, ABT_pool& pool,
                hg_id_t rpc_id);

void run_my_minimal_rpc(margo_instance_id& margo_id, hg_class_t* hg_class, hg_context_t* hg_context, hg_id_t rpc_id);

hg_id_t register_client_rpcs_test(hg_class_t* hg_class);

#endif //LFS_INIT_HPP
