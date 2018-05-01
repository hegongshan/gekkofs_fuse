
#ifndef IFS_METADENTRY_HPP
#define IFS_METADENTRY_HPP

#include <daemon/adafs_daemon.hpp>
#include <daemon/classes/metadata.hpp>

int create_node(const std::string& path, const uid_t uid, const gid_t gid, mode_t mode);

void create_metadentry(const std::string& path, mode_t mode);

int db_val_to_stat(const std::string& path, std::string db_val, struct stat& attr);

std::string get_metadentry_str(const std::string& path);

Metadata get_metadentry(const std::string& path);

void remove_node(const std::string& path);

size_t get_metadentry_size(const std::string& path);

int update_metadentry_size(const std::string& path, size_t io_size, off_t offset, bool append, size_t& read_size);

void update_metadentry(const std::string& path, Metadata& md);

int check_access_mask(const std::string& path, int mask);

#endif //IFS_METADENTRY_HPP
