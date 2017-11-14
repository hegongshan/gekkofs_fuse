
#include <rpc/rpc_types.hpp>
#include "rpc/sender/c_data.hpp"

using namespace std;


int rpc_send_write(const size_t recipient, const string& path, const size_t in_size, const off_t in_offset,
                   const char* buf, size_t& write_size, const bool append) {

    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_write_data_in_t in;
    rpc_data_out_t out;
    int err;
    // fill in
    in.path = path.c_str();
    in.size = in_size;
    in.offset = in_offset;
    if (append)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;
    // TODO HG_ADDR_T is never freed atm. Need to change LRUCache
    if (!RPC_DATA->get_addr_by_hostid(recipient, svr_addr)) {
        ADAFS_DATA->spdlogger()->error("server address not resolvable for host id {}", recipient);
        return 1;
    }
    auto ret = HG_Create(RPC_DATA->client_hg_context(), svr_addr, RPC_DATA->rpc_srv_write_data_id(), &handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("creating handle FAILED");
        return 1;
    }
    auto hgi = HG_Get_info(handle);
    /* register local target buffer for bulk access */
    // this is stupid. Fuse gives us a const char* but Bulk_creat expects a non-const char*.
    // The only way of getting const away from buf is to memcpy the content to a new buffer.
    // TODO better solution
    auto non_const_buf = make_unique<char[]>(in_size);
    memcpy(non_const_buf.get(), buf, in_size);
    auto b_buf = static_cast<void*>(non_const_buf.get());
    ret = HG_Bulk_create(hgi->hg_class, 1, &b_buf, &in_size, HG_BULK_READ_ONLY, &in.bulk_handle);
    if (ret != 0)
        ADAFS_DATA->spdlogger()->error("failed to create bulkd on client");

    int send_ret = HG_FALSE;
    for (int i = 0; i < max_retries; ++i) {
        send_ret = margo_forward_timed(RPC_DATA->client_mid(), handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {

        /* decode response */
        ret = HG_Get_output(handle, &out);
        err = out.res;
        write_size = static_cast<size_t>(out.io_size);
        ADAFS_DATA->spdlogger()->debug("Got response {}", out.res);
        /* clean up resources consumed by this rpc */
        HG_Free_output(handle, &out);
    } else {
        ADAFS_DATA->spdlogger()->error("RPC rpc_send_read (timed out)");
        err = EAGAIN;
    }

    in.path = nullptr;

    HG_Bulk_free(in.bulk_handle);
    HG_Free_input(handle, &in);
    HG_Destroy(handle);

    return err;
}