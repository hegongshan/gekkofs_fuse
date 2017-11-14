
#ifndef IFS_PRELOAD_C_METADENTRY_HPP
#define IFS_PRELOAD_C_METADENTRY_HPP

#include <preload/preload.hpp>
#include <rpc/rpc_types.hpp>
#include <iostream>

hg_return margo_create_wrap(const hg_id_t ipc_id, const hg_id_t rpc_id, const std::string& path, hg_handle_t& handle,
                            hg_addr_t& svr_addr);

void send_minimal_rpc(const hg_id_t minimal_id);

int rpc_send_open(const hg_id_t ipc_open_id, const hg_id_t rpc_open_id, const std::string& path, const mode_t mode,
                  const int flags);

int
rpc_send_stat(const hg_id_t ipc_stat_id, const hg_id_t rpc_stat_id, const std::string& path, string& attr);

int rpc_send_unlink(const hg_id_t ipc_unlink_id, const hg_id_t rpc_unlink_id, const std::string& path);

int rpc_send_update_metadentry(const hg_id_t ipc_update_metadentry_id, const hg_id_t rpc_update_metadentry_id,
                               const string& path, const Metadentry& md, const MetadentryUpdateFlags& md_flags);

int rpc_send_update_metadentry_size(const hg_id_t ipc_update_metadentry_size_id,
                                    const hg_id_t rpc_update_metadentry_size_id, const string& path, const off_t size,
                                    const bool append_flag, off_t& ret_size);

#endif //IFS_PRELOAD_C_METADENTRY_HPP
