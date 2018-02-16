
#ifndef IFS_PRELOAD_C_METADENTRY_HPP
#define IFS_PRELOAD_C_METADENTRY_HPP

#include <preload/preload.hpp>
#include <rpc/rpc_types.hpp>
#include <iostream>

inline hg_return_t margo_forward_timed_wrap_timer(hg_handle_t& handle, void* in_struct, const char* func);

inline hg_return_t margo_forward_timed_wrap(hg_handle_t& handle, void* in_struct);

void send_minimal_rpc(hg_id_t minimal_id);

int rpc_send_mk_node(const std::string& path, const mode_t mode);

int rpc_send_access(const std::string& path, const int mask);

int rpc_send_stat(const std::string& path, std::string& attr);

int rpc_send_rm_node(const std::string& path);

int rpc_send_update_metadentry(const std::string& path, const Metadentry& md, const MetadentryUpdateFlags& md_flags);

int rpc_send_update_metadentry_size(const std::string& path, size_t size, off_t offset, bool append_flag,
                                    off_t& ret_size);

int rpc_send_get_metadentry_size(const std::string& path, off_t& ret_size);

#endif //IFS_PRELOAD_C_METADENTRY_HPP
