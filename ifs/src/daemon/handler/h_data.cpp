
#include <global/rpc/rpc_types.hpp>
#include <daemon/handler/rpc_defs.hpp>
#include <daemon/adafs_ops/data.hpp>
#include <global/rpc/rpc_utils.hpp>

using namespace std;

/**
 * Determines the node id for a given path
 * @param to_hash
 * @return
 */
size_t get_rpc_node(const string& to_hash) {
    //TODO can this be a shared function?
    return std::hash<string>{}(to_hash) % ADAFS_DATA->host_size();
}


static hg_return_t rpc_srv_read_data(hg_handle_t handle) {
    rpc_read_data_in_t in{};
    rpc_data_out_t out{};
    hg_bulk_t bulk_handle = nullptr;
    // Set default out for error
    out.res = EIO;
    out.io_size = 0;
    // Getting some information from margo
    auto ret = margo_get_input(handle, &in);
    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);
    auto bulk_size = margo_bulk_get_size(in.bulk_handle);
    ADAFS_DATA->spdlogger()->debug("{}() Got read RPC (local {}) with path {} size {} offset {}", __func__,
                                   (margo_get_info(handle)->target_id == ADAFS_DATA->host_id()), in.path, bulk_size,
                                   in.offset);

    // array of pointers for bulk transfer (allocated in margo_bulk_create)
    // used for bulk transfer
    void* bulk_buf;
    // used to set pointer to offsets in bulk_buf which correspond to chunks
    vector<char*> bulk_buf_ptrs(in.chunks);
    // create bulk handle and allocated memory for buffer with buf_sizes information
    ret = margo_bulk_create(mid, 1, nullptr, &in.total_chunk_size, HG_BULK_READ_ONLY, &bulk_handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to create bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
    }
    // access the internally allocated memory buffer and put it into buf_ptrs
    uint32_t actual_count; // XXX dont need?
    ret = margo_bulk_access(bulk_handle, 0, in.total_chunk_size, HG_BULK_READWRITE, 1, &bulk_buf,
                            &in.total_chunk_size, &actual_count);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to access allocated buffer from bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }
    auto inpath = make_shared<string>(in.path);
    auto my_id = ADAFS_DATA->host_id();
    // chnk_ids used by this host
    vector<uint64_t> chnk_ids(in.chunks);
    // chnk sizes per chunk for this host
    vector<uint64_t> chnk_sizes(in.chunks);
    // local and origin offsets
    vector<uint64_t> local_offsets(in.chunks);
    vector<uint64_t> origin_offsets(in.chunks);
    // counter to track how many chunks have been assigned
    auto chnk_count = static_cast<uint64_t>(0);
    // how much is left to pull
    auto chnk_size_left = in.total_chunk_size;
    // temporary traveling pointer
    auto chnk_ptr = static_cast<char*>(bulk_buf);
    // tasks structures
    vector<ABT_eventual> task_eventuals(in.chunks);
    vector<unique_ptr<struct read_chunk_args>> task_args(in.chunks);
    auto transfer_size = (bulk_size <= CHUNKSIZE) ? bulk_size : CHUNKSIZE;
    for (auto i = in.chunk_start; i < in.chunk_end || chnk_count < in.chunks; i++) {
        if (get_rpc_node(in.path + fmt::FormatInt(i).str()) == my_id) {
            chnk_ids[chnk_count] = i; // chunk id number
            // offset case
            if (i == in.chunk_start && in.offset > 0) {
                // if only 1 destination and 1 chunk (small read) the transfer_size == bulk_size
                auto offset_transfer_size = (in.offset + bulk_size <= CHUNKSIZE) ? bulk_size : static_cast<size_t>(
                        CHUNKSIZE - in.offset);
                local_offsets[chnk_count] = 0;
                origin_offsets[chnk_count] = 0;
                bulk_buf_ptrs[chnk_count] = chnk_ptr;
                chnk_sizes[chnk_count] = offset_transfer_size;
                // util variables
                chnk_ptr += offset_transfer_size;
                chnk_size_left -= offset_transfer_size;
            } else {
                local_offsets[chnk_count] = in.total_chunk_size - chnk_size_left;
                if (in.offset > 0)
                    origin_offsets[chnk_count] = (CHUNKSIZE - in.offset) + ((i - in.chunk_start) - 1) * CHUNKSIZE;
                else
                    origin_offsets[chnk_count] = (i - in.chunk_start) * CHUNKSIZE;
                // last chunk might have different transfer_size
                if (chnk_count == in.chunks - 1)
                    transfer_size = chnk_size_left;
                bulk_buf_ptrs[chnk_count] = chnk_ptr;
                chnk_sizes[chnk_count] = transfer_size;
                // util variables
                chnk_ptr += transfer_size;
                chnk_size_left -= transfer_size;
            }
            // Starting tasklets for parallel I/O
            ABT_eventual_create(sizeof(size_t), &task_eventuals[chnk_count]); // written file return value
            auto task_arg = make_unique<read_chunk_args>();
            task_arg->path = inpath.get();
            task_arg->buf = bulk_buf_ptrs[chnk_count];
            task_arg->chnk_id = chnk_ids[chnk_count];
            task_arg->size = chnk_sizes[chnk_count];
            // only the first chunk gets the offset. the chunks are sorted on the client side
            task_arg->off = (i == 0 ? in.offset : 0);
            task_arg->eventual = task_eventuals[chnk_count];
            task_args[chnk_count] = std::move(task_arg);
            auto abt_ret = ABT_task_create(RPC_DATA->io_pool(), read_file_abt, &(*task_args[chnk_count]), nullptr);
            if (abt_ret != ABT_SUCCESS) {
                ADAFS_DATA->spdlogger()->error("{}() task create failed", __func__);
                out.res = EBUSY;
                return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
            }
            chnk_count++;
        }
    }
    for (uint64_t chnk_id = 0; chnk_id < chnk_ids.size(); chnk_id++) {
        size_t* task_read_size;
        ABT_eventual_wait(task_eventuals[chnk_id], (void**) &task_read_size);
        if (task_read_size == nullptr || *task_read_size == 0) {
            ADAFS_DATA->spdlogger()->error("{}() Reading chunk id file {} did return nothing. NO ACTION WAS DONE",
                                           __func__, chnk_id);
            // TODO How do we handle errors?
            out.io_size = 0;
            out.res = EIO;
            ADAFS_DATA->spdlogger()->error("{}() Failed to read data to local disk.");
            return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
        } else {
            ret = margo_bulk_transfer(mid, HG_BULK_PUSH, hgi->addr, in.bulk_handle, origin_offsets[chnk_id],
                                      bulk_handle, local_offsets[chnk_id], chnk_sizes[chnk_id]);
            if (ret != HG_SUCCESS) {
                ADAFS_DATA->spdlogger()->error(
                        "{}() Failed push chnkid {} on path {} to client. origin offset {} local offset {} chunk size {}",
                        __func__, chnk_id, in.path, origin_offsets[chnk_id], local_offsets[chnk_id],
                        chnk_sizes[chnk_id]);
                out.res = EBUSY;
                return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
            }
            out.io_size += *task_read_size;
        }
        ABT_eventual_free(&task_eventuals[chnk_id]);
    }

    if (in.total_chunk_size != out.io_size) {
        out.res = EIO;
        ADAFS_DATA->spdlogger()->error("{}() read chunk size does not match with requested size in path {}", __func__,
                                       in.path);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }
    // Everything is well, set result to success and send response
    out.res = 0;
    ADAFS_DATA->spdlogger()->debug("{}() Sending output response {}", __func__, out.res);
    ret = rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);

    return ret;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_read_data)

static hg_return_t rpc_srv_write_data(hg_handle_t handle) {
    /*
     * 1. Setup
     */
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    hg_bulk_t bulk_handle = nullptr;
    hg_return_t ret;
    // default out
    out.res = EIO;
    out.io_size = 0;
    // get some margo information
    margo_get_input(handle, &in);
    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_info_get_instance(hgi);
    auto segment_count = margo_bulk_get_segment_count(in.bulk_handle);
    auto bulk_size = margo_bulk_get_size(in.bulk_handle);
    ADAFS_DATA->spdlogger()->info("{}() Got write RPC (local {}) with path {} size {} offset {}", __func__,
                                  (margo_get_info(handle)->target_id == ADAFS_DATA->host_id()), in.path, bulk_size,
                                  in.offset);
    /*
     * 2. Set up buffers for pull bulk transfers
     */
    void* bulk_buf; // buffer for bulk transfer
    vector<char*> bulk_buf_ptrs(in.chunks); // buffer-chunk offsets
    // create bulk handle and allocated memory for buffer with buf_sizes information
    ret = margo_bulk_create(mid, segment_count, nullptr, &in.total_chunk_size, HG_BULK_WRITE_ONLY, &bulk_handle);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to create bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, static_cast<hg_bulk_t*>(nullptr));
    }
    // access the internally allocated memory buffer and put it into buf_ptrs
    uint32_t actual_count; // XXX dont need?
    ret = margo_bulk_access(bulk_handle, 0, in.total_chunk_size, HG_BULK_READWRITE, segment_count, &bulk_buf,
                            &in.total_chunk_size, &actual_count);
    if (ret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to access allocated buffer from bulk handle", __func__);
        return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
    }
    auto inpath = make_shared<std::string>(in.path);
    // chnk_ids used by this host
    vector<uint64_t> chnk_ids(in.chunks);
    // chnk sizes per chunk for this host
    vector<uint64_t> chnk_sizes(in.chunks);
    // counter to track how many chunks have been assigned
    auto chnk_count = static_cast<uint64_t>(0);
    // how much is left to read
    auto chnk_size_left = in.total_chunk_size;
    // temporary traveling pointer
    auto chnk_ptr = static_cast<char*>(bulk_buf);
    /*
     * consider the following cases:
     * 1. Very first chunk has offset or not and is serviced by this node
     * 2. If offset, will still be only 1 chunk written (small IO): (offset + bulk_size <= CHUNKSIZE) ? bulk_size
     * 3. If no offset, will only be 1 chunk written (small IO): (bulk_size <= CHUNKSIZE) ? bulk_size
     * 4. Chunks between start and end chunk have size of the CHUNKSIZE
     * 5. Last chunk (if multiple chunks are written): Don't write CHUNKSIZE but chnk_size_left for this destination
     *    Last chunk can also happen if only one chunk is written. This is covered by 2 and 3.
     */
    // get chunk ids that hash to this node
    auto transfer_size = (bulk_size <= CHUNKSIZE) ? bulk_size : CHUNKSIZE;
    uint64_t origin_offset;
    uint64_t local_offset;
    // task structures
    vector<ABT_eventual> task_eventuals(in.chunks);
    vector<unique_ptr<struct write_chunk_args>> task_args(in.chunks);
    for (auto chnk_idx = in.chunk_start; chnk_idx < in.chunk_end || chnk_count < in.chunks; chnk_idx++) {
        // Continue if chunk does not hash to this node
        if (get_rpc_node(in.path + fmt::FormatInt(chnk_idx).str()) != ADAFS_DATA->host_id())
            continue;
        chnk_ids[chnk_count] = chnk_idx; // chunk id number
        // offset case
        if (chnk_idx == in.chunk_start && in.offset > 0) {
            // if only 1 destination and 1 chunk (small write) the transfer_size == bulk_size
            auto offset_transfer_size = (in.offset + bulk_size <= CHUNKSIZE) ? bulk_size : static_cast<size_t>(
                    CHUNKSIZE - in.offset);
            ADAFS_DATA->spdlogger()->info(
                    "{}() BEGIN HG_BULK_PULL target_id {} origin_offset {} local_offset {} transfer_size {}",
                    __func__, hgi->target_id, 0, 0, offset_transfer_size);
            ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, 0,
                                      bulk_handle, 0, offset_transfer_size);
            ADAFS_DATA->spdlogger()->info(
                    "{}() END   HG_BULK_PULL target_id {} origin_offset {} local_offset {} transfer_size {}\n",
                    __func__, hgi->target_id, 0, 0, offset_transfer_size);
            if (ret != HG_SUCCESS) {
                ADAFS_DATA->spdlogger()->error(
                        "{}() Failed to pull data from client for chunk {} (startchunk {}; endchunk {}", __func__,
                        chnk_idx, in.chunk_start, in.chunk_end - 1);
                return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
            }
            bulk_buf_ptrs[chnk_count] = chnk_ptr;
            chnk_sizes[chnk_count] = offset_transfer_size;
            chnk_ptr += offset_transfer_size;
            chnk_size_left -= offset_transfer_size;
        } else {
            local_offset = in.total_chunk_size - chnk_size_left;
            if (in.offset > 0)
                origin_offset = (CHUNKSIZE - in.offset) + ((chnk_idx - in.chunk_start) - 1) * CHUNKSIZE;
            else
                origin_offset = (chnk_idx - in.chunk_start) * CHUNKSIZE;
            // last chunk might have different transfer_size
            if (chnk_count == in.chunks - 1)
                transfer_size = chnk_size_left;
            ADAFS_DATA->spdlogger()->info(
                    "{}() BEGIN HG_BULK_PULL target_id {} origin_offset {} local_offset {} transfer_size {}",
                    __func__, hgi->target_id, origin_offset, local_offset, transfer_size);
            ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr, in.bulk_handle, origin_offset,
                                      bulk_handle, local_offset, transfer_size);
            ADAFS_DATA->spdlogger()->info(
                    "{}() END   HG_BULK_PULL target_id {} origin_offset {} local_offset {} transfer_size {}\n",
                    __func__, hgi->target_id, origin_offset, local_offset, transfer_size);
            if (ret != HG_SUCCESS) {
                ADAFS_DATA->spdlogger()->error(
                        "{}() Failed to pull data from client for chunk {} (startchunk {}; endchunk {}", __func__,
                        chnk_idx, in.chunk_start, in.chunk_end - 1);
                return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
            }
            bulk_buf_ptrs[chnk_count] = chnk_ptr;
            chnk_sizes[chnk_count] = transfer_size;
            chnk_ptr += transfer_size;
            chnk_size_left -= transfer_size;
        }
        // Starting tasklets for parallel I/O
        ABT_eventual_create(sizeof(size_t), &task_eventuals[chnk_count]); // written file return value
        auto task_arg = make_unique<struct write_chunk_args>();
        task_arg->path = inpath.get();
        task_arg->buf = bulk_buf_ptrs[chnk_count];
        task_arg->chnk_id = chnk_ids[chnk_count];
        task_arg->size = chnk_sizes[chnk_count];
        // only the first chunk gets the offset. the chunks are sorted on the client side
        task_arg->off = (chnk_idx == 0 ? in.offset : 0);
        task_arg->eventual = task_eventuals[chnk_count];
        task_args[chnk_count] = std::move(task_arg);
        auto abt_ret = ABT_task_create(RPC_DATA->io_pool(), write_file_abt, &(*task_args[chnk_count]), nullptr);
        if (abt_ret != ABT_SUCCESS) {
            ADAFS_DATA->spdlogger()->error("{}() task create failed", __func__);
            return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
        }
        // next chunk
        chnk_count++;

    }
    for (unsigned int i = 0; i < in.chunks; i++) {
        size_t* task_written_size;
        // wait causes the calling ult to go into BLOCKED state, implicitly yielding to the pool scheduler
        ABT_eventual_wait(task_eventuals[i], (void**) &task_written_size);
        if (task_written_size == nullptr || *task_written_size == 0) {
            ADAFS_DATA->spdlogger()->error("{}() Writing file task {} did return nothing. NO ACTION WAS DONE",
                                           __func__, i);
//            // TODO How do we handle already written chunks? Ideally, we would need to remove them after failure.
            out.io_size = 0;
            ADAFS_DATA->spdlogger()->error("{}() Failed to write data to local disk.");
            return rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);
        } else {
            out.io_size += *task_written_size;
        }
        ABT_eventual_free(&task_eventuals[i]);
    }
    // XXX check that sizes left is 0 as sanity check
    // respond and cleanup
    out.res = 0;
    ADAFS_DATA->spdlogger()->debug("{}() Sending output response {}", __func__, out.res);
    ret = rpc_cleanup_respond(&handle, &in, &out, &bulk_handle);

    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_write_data)