
#ifndef IFS_PRELOAD_C_METADENTRY_HPP
#define IFS_PRELOAD_C_METADENTRY_HPP

#include <margo.h>
#include <string>

/* Forward declaration */
struct MetadentryUpdateFlags;
class OpenDir;
class Metadata;


void send_minimal_rpc(hg_id_t minimal_id);

int rpc_send_mk_node(const std::string& path, mode_t mode);

int rpc_send_access(const std::string& path, int mask);

int rpc_send_stat(const std::string& path, std::string& attr);

int rpc_send_rm_node(const std::string& path, const bool remove_metadentry_only);

int rpc_send_decr_size(const std::string& path, size_t length);

int rpc_send_update_metadentry(const std::string& path, const Metadata& md, const MetadentryUpdateFlags& md_flags);

int rpc_send_update_metadentry_size(const std::string& path, size_t size, off64_t offset, bool append_flag,
                                    off64_t& ret_size);

int rpc_send_get_metadentry_size(const std::string& path, off64_t& ret_size);

void rpc_send_get_dirents(OpenDir& open_dir);

#endif //IFS_PRELOAD_C_METADENTRY_HPP
