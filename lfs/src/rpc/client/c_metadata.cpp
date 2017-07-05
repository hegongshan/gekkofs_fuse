//
// Created by evie on 6/22/17.
//

#include "c_metadata.hpp"
#include "../rpc_types.hpp"

using namespace std;

static int max_retries = 3;

//void send_minimal_rpc() {
//void send_minimal_rpc(void* arg) {
//
//    hg_handle_t handle;
//    rpc_minimal_in_t in;
//    rpc_minimal_out_t out;
////    hg_addr_t svr_addr = HG_ADDR_NULL;
////    hg_addr_t* svr_addr = static_cast<hg_addr_t*>(arg);
//
//    hg_addr_t svr_addr = RPC_DATA->svr_addr_;
//
////    mada_addr_lookup("cci+tcp://localhost:1234"s, &svr_addr);
////    margo_addr_lookup(RPC_DATA->client_mid(), "cci+tcp://localhost:1234"s.c_str(), &svr_addr);
//
//    ADAFS_DATA->spdlogger()->debug("minimal RPC is running...");
//
//
//    /* create handle */
//    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_minimal_id(), &handle);
//    if(ret != HG_SUCCESS) {
//        printf("Creating handle FAILED\n");
//        return;
//    }
//
//    /* Send rpc. Note that we are also transmitting the bulk handle in the
//     * input struct.  It was set above.
//     */
//    in.input = 42;
//    ADAFS_DATA->spdlogger()->debug("About to call RPC ...");
//    mada_forward(handle, &in);
//
//    /* decode response */
//    ret = HG_Get_output(handle, &out);
//
//    ADAFS_DATA->spdlogger()->debug("Got response ret: {}", out.output);
//
////    HG_Addr_free(RPC_DATA->client_hg_class(), svr_addr);
//    /* clean up resources consumed by this rpc */
//    HG_Free_output(handle, &out);
//    HG_Destroy(handle);
//
//    ADAFS_DATA->spdlogger()->debug("minimal RPC is done.");
//}

void send_minimal_rpc(void* arg) {

    hg_handle_t handle;
    rpc_minimal_in_t in;
    rpc_minimal_out_t out;
    hg_addr_t svr_addr = HG_ADDR_NULL;
//    hg_addr_t* svr_addr = static_cast<hg_addr_t*>(arg);

    ADAFS_DATA->spdlogger()->debug("Looking up address");

    margo_addr_lookup(RPC_DATA->client_mid(), "cci+tcp://134.93.182.11:1234"s.c_str(), &svr_addr);

    ADAFS_DATA->spdlogger()->debug("minimal RPC is running...");


    /* create handle */
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_minimal_id(), &handle);
    if (ret != HG_SUCCESS) {
        printf("Creating handle FAILED\n");
        return;
    }

    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input = 42;
    ADAFS_DATA->spdlogger()->debug("About to call RPC ...");
    int send_ret = HG_FALSE;
    for (int i = 1; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, 5);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }

    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response ret: {}", out.output);

        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->info("RPC NOT send (timed out)");
    }
    HG_Addr_free(margo_get_class(RPC_DATA->client_mid()), svr_addr);
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    ADAFS_DATA->spdlogger()->debug("minimal RPC is done.");
}

int rpc_send_create(const size_t recipient, const fuse_ino_t parent, const string& name,
                    const uid_t uid, const gid_t gid, const mode_t mode, fuse_ino_t& new_inode) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_create_in_t in;
    rpc_create_out_t out;
    // Fill in
    in.uid = uid;
    in.gid = gid;
    in.mode = mode;
    in.filename = name.c_str();
    in.parent_inode = parent;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_create_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, 15000);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response ret: {}", out.new_inode);
        new_inode = static_cast<fuse_ino_t>(out.new_inode);

        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC send_create (timed out)");
    }


    HG_Free_input(handle, &in);
    HG_Destroy(handle);
    return 0;
}

int rpc_send_get_attr(const size_t recipient, const fuse_ino_t inode, struct stat& attr) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_get_attr_in_t in;
    rpc_get_attr_out_t out;
    // fill in
    in.inode = inode;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_attr_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }
    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, 15000);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ret = HG_Get_output(handle, &out);

        ADAFS_DATA->spdlogger()->debug("Got response mode {}", out.mode);
        attr.st_atim.tv_sec = static_cast<time_t>(out.atime);
        attr.st_mtim.tv_sec = static_cast<time_t>(out.mtime);
        attr.st_ctim.tv_sec = static_cast<time_t>(out.ctime);
        attr.st_mode = static_cast<mode_t>(out.mode);
        attr.st_uid = static_cast<uid_t>(out.uid);
        attr.st_gid = static_cast<gid_t>(out.gid);
        attr.st_nlink = static_cast<nlink_t>(out.nlink);
        attr.st_size = static_cast<size_t>(out.size);
        attr.st_blocks = static_cast<blkcnt_t>(out.blocks);

        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC send_get_attr (timed out)");
    }

    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return 0;
}