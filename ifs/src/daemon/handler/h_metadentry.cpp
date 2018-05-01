
#include <global/rpc/rpc_types.hpp>
#include <daemon/handler/rpc_defs.hpp>
#include <daemon/backend/metadata/db.hpp>

#include <daemon/adafs_ops/metadentry.hpp>

using namespace std;

static hg_return_t rpc_minimal(hg_handle_t handle) {
    rpc_minimal_in_t in{};
    rpc_minimal_out_t out{};
    // Get input
    auto ret = margo_get_input(handle, &in);
    assert(ret == HG_SUCCESS);

    ADAFS_DATA->spdlogger()->debug("Got simple RPC with input {}", in.input);

    // Create output and send it
    out.output = in.input * 2;
    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.output);
    auto hret = margo_respond(handle, &out);
    assert(hret == HG_SUCCESS);

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    ADAFS_DATA->spdlogger()->debug("Done with minimal rpc handler!");

    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_minimal)

static hg_return_t rpc_srv_mk_node(hg_handle_t handle) {
    rpc_mk_node_in_t in{};
    rpc_err_out_t out{};

    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("{}() Got RPC (from local {}) with path {}", __func__,
                                   (margo_get_info(handle)->context_id == ADAFS_DATA->host_id()), in.path);
    // create metadentry
    out.err = create_metadentry(in.path, in.mode);

    ADAFS_DATA->spdlogger()->debug("{}() Sending output err {}", __func__, out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond");
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_mk_node)

static hg_return_t rpc_srv_access(hg_handle_t handle) {
    rpc_access_in_t in{};
    rpc_err_out_t out{};

    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("{}() Got RPC (from local {}) with path {}", __func__,
                                   (margo_get_info(handle)->context_id == ADAFS_DATA->host_id()), in.path);
    // access metadentry
    out.err = check_access_mask(in.path, in.mask);

    ADAFS_DATA->spdlogger()->debug("{}() Sending output err {}", __func__, out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond");
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_access)

static hg_return_t rpc_srv_stat(hg_handle_t handle) {
    rpc_path_only_in_t in{};
    rpc_stat_out_t out{};
    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got srv stat RPC for path {}", in.path);
    std::string val;

    try {
        // get the metadata
        val = get_metadentry_str(in.path);
        out.db_val = val.c_str();
        out.err = 0;
        ADAFS_DATA->spdlogger()->debug("{}() Sending output mode {}", __func__, out.db_val);
    } catch (const NotFoundException& e) {
        ADAFS_DATA->spdlogger()->debug("{}() Entry not found: {}", __func__, in.path);
        out.err = ENOENT;
    } catch (const std::exception& e) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to get metadentry from DB: {}", __func__, e.what());
        out.err = EBUSY;
    }

    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond", __func__);
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_stat)

static hg_return_t rpc_srv_rm_node(hg_handle_t handle) {
    rpc_rm_node_in_t in{};
    rpc_err_out_t out{};

    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got remove node RPC with path {}", in.path);

    try {
        // Remove metadentry if exists on the node
        // and remove all chunks for that file
        remove_node(in.path);
        out.err = 0;
    } catch (const NotFoundException& e) {
        /* The metadentry was not found on this node,
         * this is not an error. At least one node involved in this
         * broadcast operation will find and delete the entry on its local
         * MetadataDB.
         * TODO: send the metadentry remove only to the node that actually
         * has it.
         */
        out.err = 0;
    } catch (const std::exception& e) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to remove node: {}", __func__, e.what());
        out.err = EBUSY;
    }

    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond");
    }
    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_rm_node)


static hg_return_t rpc_srv_update_metadentry(hg_handle_t handle) {
    rpc_update_metadentry_in_t in{};
    rpc_err_out_t out{};


    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got update metadentry RPC with path {}", in.path);

    // do update
    try {
        Metadata md = get_metadentry(in.path);
        out.err = 0;
        if (in.inode_no_flag == HG_TRUE)
            md.inode_no(in.inode_no);
        if (in.block_flag == HG_TRUE)
            md.blocks(in.blocks);
        if (in.uid_flag == HG_TRUE)
            md.uid(in.uid);
        if (in.gid_flag == HG_TRUE)
            md.gid(in.gid);
        if (in.nlink_flag == HG_TRUE)
            md.link_count(in.nlink);
        if (in.size_flag == HG_TRUE)
            md.size(in.size);
        if (in.atime_flag == HG_TRUE)
            md.atime(in.atime);
        if (in.mtime_flag == HG_TRUE)
            md.mtime(in.mtime);
        if (in.ctime_flag == HG_TRUE)
            md.ctime(in.ctime);
        out.err = update_metadentry(in.path, md);
    } catch (const std::exception& e){
        //TODO handle NotFoundException
        ADAFS_DATA->spdlogger()->error("{}() Failed to update entry", __func__);
        out.err = 1;
    }

    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond");
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_update_metadentry)

static hg_return_t rpc_srv_update_metadentry_size(hg_handle_t handle) {
    rpc_update_metadentry_size_in_t in{};
    rpc_update_metadentry_size_out_t out{};


    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("{}() Got update metadentry size RPC with path {}", __func__, in.path);

    // do update
    size_t read_size;
    auto err = update_metadentry_size(in.path, in.size, in.offset, (in.append == HG_TRUE), read_size);
    if (err == 0) {
        out.err = 0;
        out.ret_size = read_size;
    } else {
        out.err = err;
        out.ret_size = 0;
    }
    ADAFS_DATA->spdlogger()->debug("{}() Sending output {}", __func__, out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond", __func__);
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_update_metadentry_size)

static hg_return_t rpc_srv_get_metadentry_size(hg_handle_t handle) {
    rpc_path_only_in_t in{};
    rpc_get_metadentry_size_out_t out{};


    auto ret = margo_get_input(handle, &in);
    if (ret != HG_SUCCESS)
        ADAFS_DATA->spdlogger()->error("{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    ADAFS_DATA->spdlogger()->debug("Got update metadentry size RPC with path {}", in.path);

    // do update
    try {
        out.ret_size = get_metadentry_size(in.path);
        out.err = 0;
    } catch (const NotFoundException& e) {
        ADAFS_DATA->spdlogger()->debug("{}() Entry not found: {}", in.path);
        out.err = ENOENT;
    } catch (const std::exception& e) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to get metadentry size from DB: {}", e.what());
        out.err = EBUSY;
    }

    ADAFS_DATA->spdlogger()->debug("Sending output {}", out.err);
    auto hret = margo_respond(handle, &out);
    if (hret != HG_SUCCESS) {
        ADAFS_DATA->spdlogger()->error("{}() Failed to respond");
    }

    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

DEFINE_MARGO_RPC_HANDLER(rpc_srv_get_metadentry_size)