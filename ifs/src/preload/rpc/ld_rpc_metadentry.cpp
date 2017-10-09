//
// Created by evie on 9/7/17.
//

#include <preload/rpc/ld_rpc_metadentry.hpp>


using namespace std;

static int max_retries = 3;

void send_minimal_rpc(const hg_id_t minimal_id) {

//    hg_handle_t handle;
//    rpc_minimal_in_t in;
//    rpc_minimal_out_t out;
//    hg_addr_t svr_addr = HG_ADDR_NULL;
////    hg_addr_t* svr_addr = static_cast<hg_addr_t*>(arg);
//
//    ADAFS_DATA->spdlogger()->debug("Looking up address");
//
//    margo_addr_lookup(RPC_DATA->client_mid(), "bmi+tcp://134.93.182.11:1234"s.c_str(), &svr_addr);
//
//    ADAFS_DATA->spdlogger()->debug("minimal RPC is running...");
//
//
//    /* create handle */
//    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_minimal_id(), &handle);
//    if (ret != HG_SUCCESS) {
//        printf("Creating handle FAILED\n");
//        return;
//    }
//
//    /* Send rpc. Note that we are also transmitting the bulk handle in the
//     * input struct.  It was set above.
//     */
//    in.input = 42;
//    ADAFS_DATA->spdlogger()->debug("About to call RPC ...");
//    int send_ret = HG_FALSE;
//    for (int i = 1; i < max_retries; ++i) {
//        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, 5);
//        if (send_ret == HG_SUCCESS) {
//            break;
//        }
//    }
//
//    if (send_ret == HG_SUCCESS) {
//        /* decode response */
//        ret = HG_Get_output(handle, &out);
//
//        ADAFS_DATA->spdlogger()->debug("Got response ret: {}", out.output);
//
//        /* clean up resources consumed by this rpc */
//        HG_Free_output(handle, &out);
//    } else {
//        ADAFS_DATA->spdlogger()->info("RPC NOT send (timed out)");
//    }
//    HG_Addr_free(margo_get_class(RPC_DATA->client_mid()), svr_addr);
//    HG_Free_input(handle, &in);
//    HG_Destroy(handle);
//
//    ADAFS_DATA->spdlogger()->debug("minimal RPC is done.");
}

int rpc_send_create_node(const hg_id_t rpc_create_node_id, const size_t recipient, const std::string& path,
                         const mode_t mode) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_create_node_in_t in;
    rpc_err_out_t out;
    // fill in
    in.path = path.c_str();
    in.mode = mode;
    int err = EUNKNOWN;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!get_addr_by_hostid(recipient, svr_addr)) {
        LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %lu\n", recipient);
        return 1;
    }
    auto ret = HG_Create(margo_get_context(ld_margo_rpc_id()), svr_addr, rpc_create_node_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    int send_ret = HG_FALSE;
    LD_LOG_TRACE0(debug_fd, "About to send create_node RPC ...\n");
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_rpc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        LD_LOG_TRACE(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC send_create_node (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return err;
}

string rpc_send_get_attr(const size_t recipient, const std::string& path) {
//    hg_handle_t handle;
//    hg_addr_t svr_addr = HG_ADDR_NULL;
//    rpc_get_attr_in_t in;
//    rpc_get_attr_out_t out;
//    // fill in
//    in.path = path.c_str();
//    hg_bool_t success = HG_FALSE;
//    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
//    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
//        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
//        return ""s;
//    }
//    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_attr_id(), &handle);
//    if (ret != HG_SUCCESS) {
//        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
//        return ""s;
//    }
//    int send_ret = HG_FALSE;
//    for (int i = 0; i < max_retries; ++i) {
//        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, RPC_TIMEOUT);
//        if (send_ret == HG_SUCCESS) {
//            break;
//        }
//    }
//    if (send_ret == HG_SUCCESS) {
//        /* decode response */
//        ret = HG_Get_output(handle, &out);
//
//        ADAFS_DATA->spdlogger()->debug("Got response success: {}", out.db_val);
//
//        if (out.db_val == ""s) {
//            success = HG_FALSE;
//        } else {
//            success = HG_TRUE;
////            db_val_to_stat(path, out.db_val, *attr);
//        }
//
//        /* clean up resources consumed by this rpc */
//        HG_Free_output(handle, &out);
//    } else {
//        ADAFS_DATA->spdlogger()->error("RPC get_attr (timed out)");
//    }
//    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!
//
//    HG_Free_input(handle, &in);
//    HG_Destroy(handle);
//    return success == HG_TRUE ? string(out.db_val) : nullptr;
    return "";
}

int rpc_send_remove_node(const hg_id_t rpc_remove_node_id, const size_t recipient, const std::string& path) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_remove_node_in_t in;
    rpc_err_out_t out;
    // fill in
    in.path = path.c_str();
    int err = EUNKNOWN;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!get_addr_by_hostid(recipient, svr_addr)) {
        LD_LOG_ERROR(debug_fd, "server address not resolvable for host id %d\n", static_cast<int>(recipient));
        return 1;
    }
    auto ret = HG_Create(margo_get_context(ld_margo_rpc_id()), svr_addr, rpc_remove_node_id, &handle);
    if (ret != HG_SUCCESS) {
        LD_LOG_ERROR0(debug_fd, "creating handle FAILED\n");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(ld_margo_rpc_id(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        LD_LOG_DEBUG(debug_fd, "Got response success: %d\n", out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        LD_LOG_ERROR0(debug_fd, "RPC send_remove_node (timed out)\n");
    }

    in.path = nullptr; // XXX temporary. If this is not done free input crashes because of invalid pointer?!

    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return err;
}