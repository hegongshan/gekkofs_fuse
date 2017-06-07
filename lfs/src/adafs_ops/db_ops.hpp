//
// Created by evie on 6/6/17.
//

#ifndef LFS_DB_OPS_HPP
#define LFS_DB_OPS_HPP

#include "../main.hpp"


bool db_put(const std::string key, const time_t val);

time_t db_get(const std::string key);

#endif //LFS_DB_OPS_HPP
