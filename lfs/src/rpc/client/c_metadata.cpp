//
// Created by evie on 6/22/17.
//

#include "c_metadata.hpp"

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