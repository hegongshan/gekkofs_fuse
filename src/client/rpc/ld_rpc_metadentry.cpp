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
#include <client/rpc/ld_rpc_metadentry.hpp>
#include "client/preload.hpp"
#include "client/preload_util.hpp"
#include "client/open_dir.hpp"
#include <global/rpc/rpc_utils.hpp>
#include <global/rpc/distributor.hpp>
#include <global/rpc/rpc_types.hpp>
#include <client/rpc/hg_rpcs.hpp>

namespace rpc_send  {

using namespace std;

static inline hg_return_t
margo_forward_timed_wrap(const hg_handle_t& handle, void* in_struct) {
    return margo_forward_timed(handle, in_struct, RPC_TIMEOUT);
}

int mk_node(const std::string& path, const mode_t mode) {

    int err = EUNKNOWN;
    auto endp = CTX->hosts2().at(
            CTX->distributor()->locate_file_metadata(path));
    
    try {
        CTX->log()->debug("{}() Sending RPC ...", __func__);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we can
        // retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint) 
        // returning one result and a broadcast(endpoint_set) returning a 
        // result_set. When that happens we can remove the .at(0) :/
        auto out = 
            ld_network_service->post<gkfs::rpc::create>(endp, path, mode).get().at(0);
        err = out.err();
        CTX->log()->debug("{}() Got response success: {}", __func__, err);

    } catch(const std::exception& ex) {
        CTX->log()->error("{}() while getting rpc output", __func__);
        errno = EBUSY;
        return -1;
    }

    return err;
}

int stat(const std::string& path, string& attr) {

    auto endp = CTX->hosts2().at(
            CTX->distributor()->locate_file_metadata(path));
    
    try {
        CTX->log()->debug("{}() Sending RPC ...", __func__);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we can
        // retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint) 
        // returning one result and a broadcast(endpoint_set) returning a 
        // result_set. When that happens we can remove the .at(0) :/
        auto out = 
            ld_network_service->post<gkfs::rpc::stat>(endp, path).get().at(0);
        CTX->log()->debug("{}() Got response success: {}", __func__, out.err());

        if(out.err() != 0) {
            errno = out.err();
            return -1;
        }

        attr = out.db_val();
        return 0;

    } catch(const std::exception& ex) {
        CTX->log()->error("{}() while getting rpc output", __func__);
        errno = EBUSY;
        return -1;
    }

    return 0;
}

int decr_size(const std::string& path, size_t length) {

    auto endp = CTX->hosts2().at(
        CTX->distributor()->locate_file_metadata(path));

    try {

        CTX->log()->debug("{}() Sending RPC ...", __func__);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we can
        // retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint) 
        // returning one result and a broadcast(endpoint_set) returning a 
        // result_set. When that happens we can remove the .at(0) :/
        auto out = 
            ld_network_service->post<gkfs::rpc::decr_size>(
                    endp, path, length).get().at(0);

        CTX->log()->debug("{}() Got response success: {}", __func__, out.err());

        if(out.err() != 0) {
            errno = out.err();
            return -1;
        }

        return 0;

    } catch(const std::exception& ex) {
        CTX->log()->error("{}() while getting rpc output", __func__);
        errno = EBUSY;
        return -1;
    }
}

int rm_node(const std::string& path, const bool remove_metadentry_only) {

    // if only the metadentry should be removed, send one rpc to the
    // metadentry's responsible node to remove the metadata
    // else, send an rpc to all hosts and thus broadcast chunk_removal.
    if(remove_metadentry_only) {

        auto endp = CTX->hosts2().at(
            CTX->distributor()->locate_file_metadata(path));

        try {

            CTX->log()->debug("{}() Sending RPC ...", __func__);
            // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we can
            // retry for RPC_TRIES (see old commits with margo)
            // TODO(amiranda): hermes will eventually provide a post(endpoint) 
            // returning one result and a broadcast(endpoint_set) returning a 
            // result_set. When that happens we can remove the .at(0) :/
            auto out = 
                ld_network_service->post<gkfs::rpc::remove>(endp, path).get().at(0);

            CTX->log()->debug("{}() Got response success: {}", __func__, out.err());

            if(out.err() != 0) {
                errno = out.err();
                return -1;
            }
            
            return 0;

        } catch(const std::exception& ex) {
            CTX->log()->error("{}() while getting rpc output", __func__);
            errno = EBUSY;
            return -1;
        }

        return 0;
    }

    std::vector<hermes::rpc_handle<gkfs::rpc::remove>> handles;

    hermes::endpoint_set endps;

    std::copy(CTX->hosts2().begin(), 
              CTX->hosts2().end(), 
              std::back_inserter(endps));

    try {

        auto output_set = 
            ld_network_service->broadcast<gkfs::rpc::remove>(endps, path).get();

        // Wait for RPC responses and then get response
        for (const auto& out : output_set) {
            CTX->log()->debug("{}() Got response success: {}", __func__, out.err());

            if(out.err() != 0) {
                errno = out.err();
                return -1;
            }
        }

        return 0;

    } catch(const std::exception& ex) {
        CTX->log()->error("{}() while getting rpc output", __func__);
        errno = EBUSY;
        return -1;
    }
}


int update_metadentry(const string& path, const Metadata& md, const MetadentryUpdateFlags& md_flags) {

    auto endp = CTX->hosts2().at(
        CTX->distributor()->locate_file_metadata(path));

    try {

        CTX->log()->debug("{}() Sending RPC ...", __func__);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we can
        // retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint) 
        // returning one result and a broadcast(endpoint_set) returning a 
        // result_set. When that happens we can remove the .at(0) :/
        auto out = 
            ld_network_service->post<gkfs::rpc::update_metadentry>(
                    endp, 
                    path,
                    (md_flags.link_count ? md.link_count() : 0),
                    /* mode */ 0,
                    /* uid */  0,
                    /* gid */  0,
                    (md_flags.size ? md.size() : 0),
                    (md_flags.blocks ? md.blocks() : 0),
                    (md_flags.atime ? md.atime() : 0),
                    (md_flags.mtime ? md.mtime() : 0),
                    (md_flags.ctime ? md.ctime() : 0),
                    bool_to_merc_bool(md_flags.link_count),
                    /* mode_flag */ false, 
                    bool_to_merc_bool(md_flags.size),
                    bool_to_merc_bool(md_flags.blocks),
                    bool_to_merc_bool(md_flags.atime),
                    bool_to_merc_bool(md_flags.mtime),
                    bool_to_merc_bool(md_flags.ctime)).get().at(0);

        CTX->log()->debug("{}() Got response success: {}", __func__, out.err());

        if(out.err() != 0) {
            errno = out.err();
            return -1;
        }

        return 0;

    } catch(const std::exception& ex) {
        CTX->log()->error("{}() while getting rpc output", __func__);
        errno = EBUSY;
        return -1;
    }
}

int update_metadentry_size(const string& path, const size_t size, const off64_t offset, const bool append_flag,
                                    off64_t& ret_size) {
    hg_handle_t handle;
    rpc_update_metadentry_size_in_t in{};
    rpc_update_metadentry_size_out_t out{};
    // add data
    in.path = path.c_str();
    in.size = size;
    in.offset = offset;
    if (append_flag)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;
    int err = EUNKNOWN;

    CTX->log()->debug("{}() Creating Mercury handle ...", __func__);
    auto ret = margo_create_wrap(rpc_update_metadentry_size_id, path, handle);
    if (ret != HG_SUCCESS) {
        ret_size = 0;
        errno = EBUSY;
        margo_destroy(handle);
        return -1;
    }
    // Send rpc
    ret = margo_forward_timed_wrap(handle, &in);
    if (ret != HG_SUCCESS) {
        CTX->log()->error("{}() margo forward failed: {}", __func__, HG_Error_to_string(ret));
        ret_size = 0;
        errno = EBUSY;
        margo_destroy(handle);
        return -1;
    }

    ret = margo_get_output(handle, &out);
    if (ret != HG_SUCCESS) {
        CTX->log()->error("{}() failed to get rpc ouptut: {}", __func__, HG_Error_to_string(ret));
        ret_size = 0;
        errno = EBUSY;
        margo_free_output(handle, &out);
        margo_destroy(handle);
    }

    CTX->log()->debug("{}() Got response: {}", __func__, out.err);
    err = out.err;
    ret_size = out.ret_size;

    margo_free_output(handle, &out);
    margo_destroy(handle);
    return err;
}

int get_metadentry_size(const std::string& path, off64_t& ret_size) {

    auto endp = CTX->hosts2().at(
        CTX->distributor()->locate_file_metadata(path));

    try {

        CTX->log()->debug("{}() Sending RPC ...", __func__);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we can
        // retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint) 
        // returning one result and a broadcast(endpoint_set) returning a 
        // result_set. When that happens we can remove the .at(0) :/
        auto out = 
            ld_network_service->post<gkfs::rpc::get_metadentry_size>(
                    endp, path).get().at(0);

        CTX->log()->debug("{}() Got response success: {}", __func__, out.err());

        ret_size = out.ret_size();
        return out.err();

    } catch(const std::exception& ex) {
        CTX->log()->error("{}() while getting rpc output", __func__);
        errno = EBUSY;
        ret_size = 0;
        return EUNKNOWN;
    }
}

/**
 * Sends an RPC request to a specific node to push all chunks that belong to him
 */
void get_dirents(OpenDir& open_dir){
    CTX->log()->trace("{}() called", __func__);
    auto const root_dir = open_dir.path();
    auto const targets = CTX->distributor()->locate_directory_metadata(root_dir);
    auto const host_size = targets.size();
    std::vector<hg_handle_t> rpc_handles(host_size);
    std::vector<margo_request> rpc_waiters(host_size);
    std::vector<rpc_get_dirents_in_t> rpc_in(host_size);
    std::vector<char*> recv_buffers(host_size);

    /* preallocate receiving buffer. The actual size is not known yet.
     *
     * On C++14 make_unique function also zeroes the newly allocated buffer.
     * It turns out that this operation is increadibly slow for such a big
     * buffer. Moreover we don't need a zeroed buffer here.
     */
    auto recv_buff = std::unique_ptr<char[]>(new char[RPC_DIRENTS_BUFF_SIZE]);
    const unsigned long int per_host_buff_size = RPC_DIRENTS_BUFF_SIZE / host_size;

    hg_return_t hg_ret;

    for(const auto& target_host: targets){

        CTX->log()->trace("{}() target_host: {}", __func__, target_host);
        //Setup rpc input parameters for each host
        rpc_in[target_host].path = root_dir.c_str();
        recv_buffers[target_host] = recv_buff.get() + (target_host * per_host_buff_size);

        hg_ret = margo_bulk_create(
                    ld_margo_rpc_id, 1,
                    reinterpret_cast<void**>(&recv_buffers[target_host]),
                    &per_host_buff_size,
                    HG_BULK_WRITE_ONLY, &(rpc_in[target_host].bulk_handle));
        if(hg_ret != HG_SUCCESS){
            throw std::runtime_error("Failed to create margo bulk handle");
        }

        hg_ret = margo_create_wrap_helper(rpc_get_dirents_id, target_host, rpc_handles[target_host]);
        if (hg_ret != HG_SUCCESS) {
            std::runtime_error("Failed to create margo handle");
        }
        // Send RPC
        CTX->log()->trace("{}() Sending RPC to host: {}", __func__, target_host);
        hg_ret = margo_iforward(rpc_handles[target_host],
                             &rpc_in[target_host],
                             &rpc_waiters[target_host]);
        if (hg_ret != HG_SUCCESS) {
            CTX->log()->error("{}() Unable to send non-blocking get_dirents on {} to recipient {}", __func__, root_dir, target_host);
            for (uint64_t i = 0; i <= target_host; i++) {
                margo_bulk_free(rpc_in[i].bulk_handle);
                margo_destroy(rpc_handles[i]);
            }
            throw std::runtime_error("Failed to forward non-blocking rpc request");
        }
    }

    for(unsigned int target_host = 0; target_host < host_size; target_host++){
        hg_ret = margo_wait(rpc_waiters[target_host]);
        if (hg_ret != HG_SUCCESS) {
            throw std::runtime_error(fmt::format("Failed while waiting for rpc completion. [root dir: {}, target host: {}]", root_dir, target_host));
        }
        rpc_get_dirents_out_t out{};
        hg_ret = margo_get_output(rpc_handles[target_host], &out);
        if (hg_ret != HG_SUCCESS) {
            throw std::runtime_error(fmt::format("Failed to get rpc output.. [path: {}, target host: {}]", root_dir, target_host));
        }

        if (out.err) {
            CTX->log()->error("{}() Sending RPC to host: {}", __func__, target_host);
            throw std::runtime_error(fmt::format("Failed to retrieve dir entries from host '{}'. "
                                                 "Error '{}', path '{}'", target_host, strerror(out.err), root_dir));
        }
        bool* bool_ptr = reinterpret_cast<bool*>(recv_buffers[target_host]);
        char* names_ptr = recv_buffers[target_host] + (out.dirents_size * sizeof(bool));

        for(unsigned int i = 0; i < out.dirents_size; i++){

            FileType ftype = (*bool_ptr)? FileType::directory : FileType::regular;
            bool_ptr++;

            //Check that we are not outside the recv_buff for this specific host
            assert((names_ptr - recv_buffers[target_host]) > 0);
            assert(static_cast<unsigned long int>(names_ptr - recv_buffers[target_host]) < per_host_buff_size);

            auto name = std::string(names_ptr);
            names_ptr += name.size() + 1;

            open_dir.add(name, ftype);
        }

        margo_free_output(rpc_handles[target_host], &out);
        margo_bulk_free(rpc_in[target_host].bulk_handle);
        margo_destroy(rpc_handles[target_host]);
    }
}

#ifdef HAS_SYMLINKS

int mk_symlink(const std::string& path, const std::string& target_path) {
    hg_handle_t handle;
    rpc_mk_symlink_in_t in{};
    rpc_err_out_t out{};
    int err = EUNKNOWN;
    // fill in
    in.path = path.c_str();
    in.target_path = target_path.c_str();
    // Create handle
    CTX->log()->debug("{}() Creating Mercury handle ...", __func__);
    auto ret = margo_create_wrap(rpc_mk_symlink_id, path, handle);
    if (ret != HG_SUCCESS) {
        errno = EBUSY;
        return -1;
    }
    // Send rpc
    CTX->log()->debug("{}() About to send RPC ...", __func__);
    ret = margo_forward_timed_wrap(handle, &in);
    // Get response
    if (ret == HG_SUCCESS) {
        CTX->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);
        if (ret == HG_SUCCESS) {
            CTX->log()->debug("{}() Got response success: {}", __func__, out.err);
            err = out.err;
        } else {
            // something is wrong
            errno = EBUSY;
            CTX->log()->error("{}() while getting rpc output", __func__);
        }
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        CTX->log()->warn("{}() timed out");
        errno = EBUSY;
    }
    margo_destroy(handle);
    return err;
}

#endif

} //end namespace rpc_send
