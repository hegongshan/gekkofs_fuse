
#ifndef IFS_PRELOAD_C_METADENTRY_HPP
#define IFS_PRELOAD_C_METADENTRY_HPP

#include <preload/preload.hpp>
#include <rpc/rpc_types.hpp>
#include <iostream>

void send_minimal_rpc(hg_id_t minimal_id);

int rpc_send_open(const std::string& path, mode_t mode, int flags);

int rpc_send_stat(const std::string& path, std::string& attr);

int rpc_send_unlink(const std::string& path);

int rpc_send_update_metadentry(const std::string& path, const Metadentry& md, const MetadentryUpdateFlags& md_flags);

int rpc_send_update_metadentry_size(const std::string& path, size_t size, off_t offset, bool append_flag,
                                    off_t& ret_size);

#endif //IFS_PRELOAD_C_METADENTRY_HPP
