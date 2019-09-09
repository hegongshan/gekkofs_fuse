/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <global/configure.hpp>
#include <client/preload_util.hpp>
#include <client/rpc/ld_rpc_data_ws.hpp>
#include "global/rpc/rpc_types.hpp"
#include <global/rpc/distributor.hpp>
#include <global/chunk_calc_util.hpp>
#include <client/rpc/hg_rpcs.hpp>

#include <unordered_set>


namespace rpc_send {


using namespace std;

// TODO If we decide to keep this functionality with one segment, the function can be merged mostly.
// Code is mostly redundant

/**
 * Sends an RPC request to a specific node to pull all chunks that belong to him
 */
ssize_t write(const string& path, const void* buf, const bool append_flag, 
              const off64_t in_offset, const size_t write_size, 
              const int64_t updated_metadentry_size) {

    assert(write_size > 0);

    // Calculate chunkid boundaries and numbers so that daemons know in 
    // which interval to look for chunks
    off64_t offset = append_flag ? 
                        in_offset : 
                        (updated_metadentry_size - write_size);

    auto chnk_start = chnk_id_for_offset(offset, CHUNKSIZE);
    auto chnk_end = chnk_id_for_offset((offset + write_size) - 1, CHUNKSIZE);

    // Collect all chunk ids within count that have the same destination so 
    // that those are send in one rpc bulk transfer
    std::map<uint64_t, std::vector<uint64_t>> target_chnks{};
    // contains the target ids, used to access the target_chnks map. 
    // First idx is chunk with potential offset
    std::vector<uint64_t> targets{};

    // targets for the first and last chunk as they need special treatment
    uint64_t chnk_start_target = 0;
    uint64_t chnk_end_target = 0;

    for (uint64_t chnk_id = chnk_start; chnk_id <= chnk_end; chnk_id++) {
        auto target = CTX->distributor()->locate_data(path, chnk_id);

        if (target_chnks.count(target) == 0) {
            target_chnks.insert(
                    std::make_pair(target, std::vector<uint64_t>{chnk_id}));
            targets.push_back(target);
        } else {
            target_chnks[target].push_back(chnk_id);
        }

        // set first and last chnk targets
        if (chnk_id == chnk_start) {
            chnk_start_target = target;
        }

        if (chnk_id == chnk_end) {
            chnk_end_target = target;
        }
    }

    // some helper variables for async RPC
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{const_cast<void*>(buf), write_size},
    };

    // expose user buffers so that they can serve as RDMA data sources
    // (these are automatically "unexposed" when the destructor is called)
    hermes::exposed_memory local_buffers;

    try {
        local_buffers = 
            ld_network_service->expose(bufseq, hermes::access_mode::read_only);

    } catch (const std::exception& ex) {
        CTX->log()->error("{}() Failed to expose buffers for RMA", __func__);
        errno = EBUSY;
        return -1;
    }

    std::vector<hermes::rpc_handle<gkfs::rpc::write_data>> handles;

    // Issue non-blocking RPC requests and wait for the result later
    //
    // TODO(amiranda): This could be simplified by adding a vector of inputs
    // to async_engine::broadcast(). This would allow us to avoid manually 
    // looping over handles as we do below
    for(const auto& target : targets) {

        // total chunk_size for target
        auto total_chunk_size = target_chnks[target].size() * CHUNKSIZE;

        // receiver of first chunk must subtract the offset from first chunk
        if (target == chnk_start_target) {
            total_chunk_size -= chnk_lpad(offset, CHUNKSIZE);
        }

        // receiver of last chunk must subtract
        if (target == chnk_end_target) {
            total_chunk_size -= chnk_rpad(offset + write_size, CHUNKSIZE);
        }

        auto endp = CTX->hosts2().at(
            CTX->distributor()->locate_file_metadata(path));

        try {

            CTX->log()->debug("{}() Sending RPC ...", __func__);

            gkfs::rpc::write_data::input in(
                path,
                // first offset in targets is the chunk with 
                // a potential offset
                chnk_lpad(offset, CHUNKSIZE),
                target,
                CTX->hosts2().size(),
                // number of chunks handled by that destination
                target_chnks[target].size(),
                // chunk start id of this write
                chnk_start,
                // chunk end id of this write
                chnk_end,
                // total size to write
                total_chunk_size,
                local_buffers);

            // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that
            // we can retry for RPC_TRIES (see old commits with margo)
            // TODO(amiranda): hermes will eventually provide a post(endpoint) 
            // returning one result and a broadcast(endpoint_set) returning a 
            // result_set. When that happens we can remove the .at(0) :/
            handles.emplace_back(
                ld_network_service->post<gkfs::rpc::write_data>(endp, in));

            CTX->log()->trace("{}() host: {}, path: {}, chunks: {}, size: {}, "
                              "offset: {}", __func__,
                              target, path, in.chunk_n(), 
                              total_chunk_size, in.offset());

        } catch(const std::exception& ex) {
            CTX->log()->error("{}() Unable to send non-blocking rpc for "
                              "path {} and recipient {}", __func__, path,
                              target);
            errno = EBUSY;
            return -1;
        }
    }

    // Wait for RPC responses and then get response and add it to out_size
    // which is the written size All potential outputs are served to free
    // resources regardless of errors, although an errorcode is set.
    bool error = false;
    ssize_t out_size = 0;
    std::size_t idx = 0;

    for(const auto& h : handles) {
        try {
            // XXX We might need a timeout here to not wait forever for an
            // output that never comes?
            auto out = h.get().at(0);

            if(out.err() != 0) {
                CTX->log()->error("{}() Daemon reported error: {}", 
                                  __func__, out.err());
                error = true;
                errno = out.err();
            }

            out_size += static_cast<size_t>(out.io_size());

        } catch(const std::exception& ex) {
            CTX->log()->error("{}() Failed to get rpc output for path {} "
                              "recipient {}", __func__, path, targets[idx]);
            error = true;
            errno = EIO;
        }

        ++idx;
    }

    return error ? -1 : out_size;
}

/**
 * Sends an RPC request to a specific node to push all chunks that belong to him
 */
ssize_t read(const string& path, void* buf, const off64_t offset, const size_t read_size) {

    // Calculate chunkid boundaries and numbers so that daemons know in which
    // interval to look for chunks
    auto chnk_start = chnk_id_for_offset(offset, CHUNKSIZE);
    auto chnk_end = chnk_id_for_offset((offset + read_size - 1), CHUNKSIZE);

    // Collect all chunk ids within count that have the same destination so 
    // that those are send in one rpc bulk transfer
    std::map<uint64_t, std::vector<uint64_t>> target_chnks{};
    // contains the recipient ids, used to access the target_chnks map. 
    // First idx is chunk with potential offset
    std::vector<uint64_t> targets{};

    // targets for the first and last chunk as they need special treatment
    uint64_t chnk_start_target = 0;
    uint64_t chnk_end_target = 0;

    for (uint64_t chnk_id = chnk_start; chnk_id <= chnk_end; chnk_id++) {
        auto target = CTX->distributor()->locate_data(path, chnk_id);

        if (target_chnks.count(target) == 0) {
            target_chnks.insert(
                    std::make_pair(target, std::vector<uint64_t>{chnk_id}));
            targets.push_back(target);
        } else {
            target_chnks[target].push_back(chnk_id);
        }

        // set first and last chnk targets
        if (chnk_id == chnk_start) {
            chnk_start_target = target;
        }

        if (chnk_id == chnk_end) {
            chnk_end_target = target;
        }
    }

    // some helper variables for async RPCs
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{buf, read_size},
    };

    // expose user buffers so that they can serve as RDMA data targets
    // (these are automatically "unexposed" when the destructor is called)
    hermes::exposed_memory local_buffers;

    try {
        local_buffers = 
            ld_network_service->expose(bufseq, hermes::access_mode::write_only);

    } catch (const std::exception& ex) {
        CTX->log()->error("{}() Failed to expose buffers for RMA", __func__);
        errno = EBUSY;
        return -1;
    }

    std::vector<hermes::rpc_handle<gkfs::rpc::read_data>> handles;

    // Issue non-blocking RPC requests and wait for the result later
    //
    // TODO(amiranda): This could be simplified by adding a vector of inputs
    // to async_engine::broadcast(). This would allow us to avoid manually 
    // looping over handles as we do below
    for(const auto& target : targets) {

        // total chunk_size for target
        auto total_chunk_size = target_chnks[target].size() * CHUNKSIZE;

        // receiver of first chunk must subtract the offset from first chunk
        if (target == chnk_start_target) {
            total_chunk_size -= chnk_lpad(offset, CHUNKSIZE);
        }

        // receiver of last chunk must subtract
        if (target == chnk_end_target) {
            total_chunk_size -= chnk_rpad(offset + read_size, CHUNKSIZE);
        }

        auto endp = CTX->hosts2().at(
            CTX->distributor()->locate_file_metadata(path));

        try {

            CTX->log()->debug("{}() Sending RPC ...", __func__);

            gkfs::rpc::read_data::input in(
                path,
                // first offset in targets is the chunk with 
                // a potential offset
                chnk_lpad(offset, CHUNKSIZE),
                target,
                CTX->hosts2().size(),
                // number of chunks handled by that destination
                target_chnks[target].size(),
                // chunk start id of this write
                chnk_start,
                // chunk end id of this write
                chnk_end,
                // total size to write
                total_chunk_size,
                local_buffers);

            // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that
            // we can retry for RPC_TRIES (see old commits with margo)
            // TODO(amiranda): hermes will eventually provide a post(endpoint) 
            // returning one result and a broadcast(endpoint_set) returning a 
            // result_set. When that happens we can remove the .at(0) :/
            handles.emplace_back(
                ld_network_service->post<gkfs::rpc::read_data>(endp, in));

            CTX->log()->trace("{}() host: {}, path: {}, chunks: {}, size: {}, "
                              "offset: {}", __func__,
                              target, path, in.chunk_n(), 
                              total_chunk_size, in.offset());

        } catch(const std::exception& ex) {
            CTX->log()->error("{}() Unable to send non-blocking rpc for "
                              "path {} and recipient {}", __func__, path,
                              target);
            errno = EBUSY;
            return -1;
        }
    }

    // Wait for RPC responses and then get response and add it to out_size
    // which is the read size. All potential outputs are served to free
    // resources regardless of errors, although an errorcode is set.
    bool error = false;
    ssize_t out_size = 0;
    std::size_t idx = 0;

    for(const auto& h : handles) {
        try {
            // XXX We might need a timeout here to not wait forever for an
            // output that never comes?
            auto out = h.get().at(0);

            if(out.err() != 0) {
                CTX->log()->error("{}() Daemon reported error: {}", 
                                  __func__, out.err());
                error = true;
                errno = out.err();
            }

            out_size += static_cast<size_t>(out.io_size());

        } catch(const std::exception& ex) {
            CTX->log()->error("{}() Failed to get rpc output for path {} "
                              "recipient {}", __func__, path, targets[idx]);
            error = true;
            errno = EIO;
        }

        ++idx;
    }

    return error ? -1 : out_size;
}

int trunc_data(const std::string& path, size_t current_size, size_t new_size) {
    assert(current_size > new_size);
    hg_return_t ret;
    rpc_trunc_in_t in;
    in.path = path.c_str();
    in.length = new_size;

    bool error = false;

    // Find out which data server needs to delete chunks in order to contact only them
    const unsigned int chunk_start = chnk_id_for_offset(new_size, CHUNKSIZE);
    const unsigned int chunk_end = chnk_id_for_offset(current_size - new_size - 1, CHUNKSIZE);
    std::unordered_set<unsigned int> hosts;
    for(unsigned int chunk_id = chunk_start; chunk_id <= chunk_end; ++chunk_id) {
        hosts.insert(CTX->distributor()->locate_data(path, chunk_id));
    }

    std::vector<hg_handle_t> rpc_handles(hosts.size());
    std::vector<margo_request> rpc_waiters(hosts.size());
    unsigned int req_num = 0;
    for (const auto& host: hosts) {
        ret = margo_create_wrap_helper(rpc_trunc_data_id, host, rpc_handles[req_num]);
        if (ret != HG_SUCCESS) {
            CTX->log()->error("{}() Unable to create Mercury handle for host: ", __func__, host);
            break;
        }

        // send async rpc
        ret = margo_iforward(rpc_handles[req_num], &in, &rpc_waiters[req_num]);
        if (ret != HG_SUCCESS) {
            CTX->log()->error("{}() Failed to send request to host: {}", __func__, host);
            break;
        }
        ++req_num;
    }

    if(req_num < hosts.size()) {
        // An error occurred. Cleanup and return
        CTX->log()->error("{}() Error -> sent only some requests {}/{}. Cancelling request...", __func__, req_num, hosts.size());
        for(unsigned int i = 0; i < req_num; ++i) {
            margo_destroy(rpc_handles[i]);
        }
        errno = EIO;
        return -1;
    }

    assert(req_num == hosts.size());
    // Wait for RPC responses and then get response
    rpc_err_out_t out{};
    for (unsigned int i = 0; i < hosts.size(); ++i) {
        ret = margo_wait(rpc_waiters[i]);
        if (ret == HG_SUCCESS) {
            ret = margo_get_output(rpc_handles[i], &out);
            if (ret == HG_SUCCESS) {
                if(out.err){
                    CTX->log()->error("{}() received error response: {}", __func__, out.err);
                    error = true;
                }
            } else {
                // Get output failed
                CTX->log()->error("{}() while getting rpc output", __func__);
                error = true;
            }
        } else {
            // Wait failed
            CTX->log()->error("{}() Failed while waiting for response", __func__);
            error = true;
        }

        /* clean up resources consumed by this rpc */
        margo_free_output(rpc_handles[i], &out);
        margo_destroy(rpc_handles[i]);
    }

    if(error) {
        errno = EIO;
        return -1;
    }
    return 0;
}

ChunkStat chunk_stat() {
    CTX->log()->trace("{}()", __func__);
    rpc_chunk_stat_in_t in;

    auto const host_size =  CTX->hosts().size();
    std::vector<hg_handle_t> rpc_handles(host_size);
    std::vector<margo_request> rpc_waiters(host_size);

    hg_return_t hg_ret;

    for (unsigned int target_host = 0; target_host < host_size; ++target_host) {
        //Setup rpc input parameters for each host
        hg_ret = margo_create_wrap_helper(rpc_chunk_stat_id, target_host,
                                          rpc_handles[target_host]);
        if (hg_ret != HG_SUCCESS) {
            throw std::runtime_error("Failed to create margo handle");
        }
        // Send RPC
        CTX->log()->trace("{}() Sending RPC to host: {}", __func__, target_host);
        hg_ret = margo_iforward(rpc_handles[target_host],
                                &in,
                                &rpc_waiters[target_host]);
        if (hg_ret != HG_SUCCESS) {
            CTX->log()->error("{}() Unable to send non-blocking chunk_stat to recipient {}", __func__, target_host);
            for (unsigned int i = 0; i <= target_host; i++) {
                margo_destroy(rpc_handles[i]);
            }
            throw std::runtime_error("Failed to forward non-blocking rpc request");
        }
    }
    unsigned long chunk_size = CHUNKSIZE;
    unsigned long chunk_total = 0;
    unsigned long chunk_free = 0;

    for (unsigned int target_host = 0; target_host < host_size; ++target_host) {
        hg_ret = margo_wait(rpc_waiters[target_host]);
        if (hg_ret != HG_SUCCESS) {
            throw std::runtime_error(fmt::format("Failed while waiting for rpc completion. target host: {}", target_host));
        }
        rpc_chunk_stat_out_t out{};
        hg_ret = margo_get_output(rpc_handles[target_host], &out);
        if (hg_ret != HG_SUCCESS) {
            throw std::runtime_error(fmt::format("Failed to get rpc output for target host: {}", target_host));
        }

        assert(out.chunk_size == chunk_size);
        chunk_total += out.chunk_total;
        chunk_free  += out.chunk_free;

        margo_free_output(rpc_handles[target_host], &out);
        margo_destroy(rpc_handles[target_host]);
    }

    return {chunk_size, chunk_total, chunk_free};
}

} // end namespace rpc_send
