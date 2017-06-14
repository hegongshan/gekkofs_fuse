//
// Created by evie on 6/14/17.
//

#include "rpcs.hpp"

using namespace std;

/* my-rpc:
 * This is an example RPC operation.  It includes a small bulk transfer,
 * driven by the server, that moves data from the client to the server.  The
 * server writes the data to a local file in /tmp.
 */

/* The rpc handler is defined as a single ULT in Argobots.  It uses
 * underlying asynchronous operations for the HG transfer, open, write, and
 * close.
 */
static void my_rpc_ult(hg_handle_t handle) {
    my_rpc_in_t in;
    my_rpc_out_t out;
    hg_bulk_t bulk_handle;
    margo_instance_id margo_id;
    const struct hg_info* hgi;

    // Receive RPC here
    auto ret = HG_Get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->info("Got RPC request with input_val: {}", in.input_val);
    out.ret = in.input_val * 2;

    /* set up target buffer for bulk transfer */
    hg_size_t size = 512;
    void* buffer;
    buffer = calloc(1, 512);

    // Register local target buffer for bulk access
    hgi = HG_Get_info(handle);
    assert(hgi);
    ret = HG_Bulk_create(hgi->hg_class, 1, &buffer, &size, HG_BULK_WRITE_ONLY, &bulk_handle);
    assert(ret == 0);

    // Retrieve the Margo instance that has been associated with a Mercury class
    margo_id = margo_hg_class_to_instance(hgi->hg_class);

    // do bulk transfer from client to server
    ret = margo_bulk_transfer(margo_id, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0, bulk_handle, 0, size);
    assert(ret == 0);
#if 1
    int fd;
    char filename[256];
#endif
    /* write to a file; would be done with abt-io if we enabled it */
#if 1
    auto aid = abt_io_init(0);
    sprintf(filename, "/tmp/margo-%d.txt", in.input_val);
    fd = abt_io_open(aid, filename, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
    assert(fd > -1);

    auto a_ret = abt_io_pwrite(aid, fd, buffer, 512, 0);
    assert(a_ret == 512);

    abt_io_close(aid, fd);
    abt_io_finalize(aid);
#endif

    HG_Free_input(handle, &in);
    auto hret = HG_Respond(handle, nullptr, nullptr, &out);
    assert (hret == HG_SUCCESS);

//    HG_Bulk_free(bulk_handle);
    HG_Destroy(handle);
    free(buffer);

    return;
}

DEFINE_MARGO_RPC_HANDLER(my_rpc_ult)

static void my_rpc_shutdown_ult(hg_handle_t handle) {
    hg_return_t hret;
    const struct hg_info* hgi;
    margo_instance_id mid;

    printf("Got RPC request to shutdown\n");

    hgi = HG_Get_info(handle);
    assert(hgi);
    mid = margo_hg_class_to_instance(hgi->hg_class);

    hret = margo_respond(mid, handle, NULL);
    assert(hret == HG_SUCCESS);

    HG_Destroy(handle);

    /* NOTE: we assume that the server daemon is using
     * margo_wait_for_finalize() to suspend until this RPC executes, so there
     * is no need to send any extra signal to notify it.
     */
    margo_finalize(mid);

    return;
}

DEFINE_MARGO_RPC_HANDLER(my_rpc_shutdown_ult)