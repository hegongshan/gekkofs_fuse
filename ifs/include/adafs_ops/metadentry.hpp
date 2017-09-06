//
// Created by evie on 9/6/17.
//

#ifndef IFS_METADENTRY_HPP
#define IFS_METADENTRY_HPP

#include "../../main.hpp"

int create_node(const std::string& path, const uid_t uid, const gid_t gid, mode_t mode);

int create_metadentry(const std::string& path, mode_t mode);

#endif //IFS_METADENTRY_HPP
