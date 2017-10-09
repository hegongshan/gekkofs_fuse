//
// Created by evie on 9/6/17.
//

#ifndef IFS_DB_OPS_HPP
#define IFS_DB_OPS_HPP

#include "../../main.hpp"

bool db_get_metadentry(const std::string& key, std::string& val);

bool db_put_metadentry(const std::string& key, const std::string& val);

bool db_delete_metadentry(const std::string& key);

bool db_metadentry_exists(const std::string& key);

bool db_is_dir_entry(const std::string& dir_path);

#endif //IFS_DB_OPS_HPP
