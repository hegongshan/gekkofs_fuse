/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GKFS_RPCS_HPP
#define GKFS_RPCS_HPP

// C includes
#include <mercury.h>
#include <mercury_proc_string.h>
#include <mercury_macros.h>

// C++ includes
#include <string>

// hermes includes
#include <hermes.hpp>

#ifndef HG_GEN_PROC_NAME
#define HG_GEN_PROC_NAME(struct_type_name) \
    hg_proc_ ## struct_type_name
#endif


#include <global/global_defs.hpp>
#include <global/rpc/rpc_types.hpp>

namespace hermes { namespace detail {

struct hg_void_t { };

static HG_INLINE hg_return_t 
hg_proc_void_t(hg_proc_t proc, void *data) 
{
    (void) proc;
    (void) data;

    return HG_SUCCESS;
}

}} // namespace hermes::detail

namespace gkfs {
namespace rpc {

//==============================================================================
// definitions for fs_config
struct fs_config {

    // forward declarations of public input/output types for this RPC
    class input;
    class output;

    // traits used so that the engine knows what to do with the RPC
    using self_type = fs_config;
    using handle_type = hermes::rpc_handle<self_type>;
    using input_type = input;
    using output_type = output;
    using mercury_input_type = hermes::detail::hg_void_t;
    using mercury_output_type = rpc_config_out_t;

    // RPC public identifier
    constexpr static const uint64_t public_id = 3033006080;

    // RPC internal Mercury identifier
    constexpr static const hg_id_t mercury_id = public_id;

    // RPC name
    constexpr static const auto name = hg_tag::fs_config;

    // requires response?
    constexpr static const auto requires_response = true;

    // Mercury callback to serialize input arguments
    constexpr static const auto mercury_in_proc_cb = 
        hermes::detail::hg_proc_void_t;

    // Mercury callback to serialize output arguments
    constexpr static const auto mercury_out_proc_cb = 
        HG_GEN_PROC_NAME(rpc_config_out_t);

    class input {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        input() { }
        input(input&& rhs) = default;
        input(const input& other) = default;
        input& operator=(input&& rhs) = default;
        input& operator=(const input& other) = default;

        explicit
        input(const hermes::detail::hg_void_t& other) { }

        explicit
        operator hermes::detail::hg_void_t() {
            return {};
        }
    };

    class output {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        output() :
            m_mountdir(),
            m_rootdir(),
            m_atime_state(),
            m_mtime_state(),
            m_ctime_state(),
            m_link_cnt_state(),
            m_blocks_state(),
            m_uid(),
            m_gid() {}

        output(const std::string& mountdir,
               const std::string& rootdir,
               bool atime_state,
               bool mtime_state,
               bool ctime_state,
               bool link_cnt_state,
               bool blocks_state,
               uint32_t uid,
               uint32_t gid) :
            m_mountdir(mountdir),
            m_rootdir(rootdir),
            m_atime_state(atime_state),
            m_mtime_state(mtime_state),
            m_ctime_state(ctime_state),
            m_link_cnt_state(link_cnt_state),
            m_blocks_state(blocks_state),
            m_uid(uid),
            m_gid(gid) {}

        output(output&& rhs) = default;
        output(const output& other) = default;
        output& operator=(output&& rhs) = default;
        output& operator=(const output& other) = default;

        explicit 
        output(const rpc_config_out_t& out) {
            m_mountdir = out.mountdir;
            m_rootdir = out.rootdir;
            m_atime_state = out.atime_state;
            m_mtime_state = out.mtime_state;
            m_ctime_state = out.ctime_state;
            m_link_cnt_state = out.link_cnt_state;
            m_blocks_state = out.blocks_state;
            m_uid = out.uid;
            m_gid = out.gid;
        }

        std::string
        mountdir() const {
            return m_mountdir;
        }

        std::string
        rootdir() const {
            return m_rootdir;
        }

        bool 
        atime_state() const {
            return m_atime_state;
        }

        bool 
        mtime_state() const {
            return m_mtime_state;
        }

        bool
        ctime_state() const {
            return m_ctime_state;
        }

        bool 
        link_cnt_state() const {
            return m_link_cnt_state;
        }

        bool 
        blocks_state() const {
            return m_blocks_state;
        }

        uint32_t 
        uid() const {
            return m_uid;
        }

        uint32_t
        gid() const {
            return m_gid;
        }

    private:
        std::string m_mountdir;
        std::string m_rootdir;
        bool m_atime_state;
        bool m_mtime_state;
        bool m_ctime_state;
        bool m_link_cnt_state;
        bool m_blocks_state;
        uint32_t m_uid;
        uint32_t m_gid;
    };
};


//==============================================================================
// definitions for create
struct create {

    // forward declarations of public input/output types for this RPC
    class input;
    class output;

    // traits used so that the engine knows what to do with the RPC
    using self_type = create;
    using handle_type = hermes::rpc_handle<self_type>;
    using input_type = input;
    using output_type = output;
    using mercury_input_type = rpc_mk_node_in_t;
    using mercury_output_type = rpc_err_out_t;

    // RPC public identifier
    constexpr static const uint64_t public_id = 796590080;

    // RPC internal Mercury identifier
    constexpr static const hg_id_t mercury_id = public_id;

    // RPC name
    constexpr static const auto name = hg_tag::create;

    // requires response?
    constexpr static const auto requires_response = true;

    // Mercury callback to serialize input arguments
    constexpr static const auto mercury_in_proc_cb = 
        HG_GEN_PROC_NAME(rpc_mk_node_in_t);

    // Mercury callback to serialize output arguments
    constexpr static const auto mercury_out_proc_cb = 
        HG_GEN_PROC_NAME(rpc_err_out_t);

    class input {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        input(const std::string& path, 
              uint32_t mode) :
            m_path(path),
            m_mode(mode) { }

        input(input&& rhs) = default;
        input(const input& other) = default;
        input& operator=(input&& rhs) = default;
        input& operator=(const input& other) = default;

        std::string
        path() const {
            return m_path;
        }

        uint32_t
        mode() const {
            return m_mode;
        }

        explicit
        input(const rpc_mk_node_in_t& other) :
            m_path(other.path),
            m_mode(other.mode) { }

        explicit
        operator rpc_mk_node_in_t() {
            return {m_path.c_str(), m_mode};
        }

    private:
        std::string m_path;
        uint32_t m_mode;
    };

    class output {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        output() :
            m_err() {}

        output(int32_t err) :
            m_err(err) {}

        output(output&& rhs) = default;
        output(const output& other) = default;
        output& operator=(output&& rhs) = default;
        output& operator=(const output& other) = default;

        explicit 
        output(const rpc_err_out_t& out) {
            m_err = out.err;
        }

        int32_t
        err() const {
            return m_err;
        }

    private:
        int32_t m_err;
    };
};

//==============================================================================
// definitions for stat
struct stat {

    // forward declarations of public input/output types for this RPC
    class input;
    class output;

    // traits used so that the engine knows what to do with the RPC
    using self_type = stat;
    using handle_type = hermes::rpc_handle<self_type>;
    using input_type = input;
    using output_type = output;
    using mercury_input_type = rpc_path_only_in_t;
    using mercury_output_type = rpc_stat_out_t;

    // RPC public identifier
    constexpr static const uint64_t public_id = 1396244480;

    // RPC internal Mercury identifier
    constexpr static const hg_id_t mercury_id = public_id;

    // RPC name
    constexpr static const auto name = hg_tag::stat;

    // requires response?
    constexpr static const auto requires_response = true;

    // Mercury callback to serialize input arguments
    constexpr static const auto mercury_in_proc_cb = 
        HG_GEN_PROC_NAME(rpc_path_only_in_t);

    // Mercury callback to serialize output arguments
    constexpr static const auto mercury_out_proc_cb = 
        HG_GEN_PROC_NAME(rpc_stat_out_t);

    class input {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        input(const std::string& path ) :
            m_path(path) { }

        input(input&& rhs) = default;
        input(const input& other) = default;
        input& operator=(input&& rhs) = default;
        input& operator=(const input& other) = default;

        std::string
        path() const {
            return m_path;
        }

        explicit
        input(const rpc_path_only_in_t& other) :
            m_path(other.path) { }

        explicit
        operator rpc_path_only_in_t() {
            return {m_path.c_str()};
        }

    private:
        std::string m_path;
    };

    class output {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        output() :
            m_err(),
            m_db_val() {}

        output(int32_t err, const std::string& db_val) :
            m_err(err),
            m_db_val(db_val) {}

        output(output&& rhs) = default;
        output(const output& other) = default;
        output& operator=(output&& rhs) = default;
        output& operator=(const output& other) = default;

        explicit 
        output(const rpc_stat_out_t& out) {
            m_err = out.err;
            m_db_val = out.db_val;
        }

        int32_t
        err() const {
            return m_err;
        }

        std::string
        db_val() const {
            return m_db_val;
        }

    private:
        int32_t m_err;
        std::string m_db_val;
    };
};

} // namespace rpc
} // namespace gkfs


#endif // GKFS_RPCS_HPP
