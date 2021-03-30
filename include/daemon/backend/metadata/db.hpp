/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS.

  GekkoFS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  GekkoFS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GEKKOFS_METADATA_DB_HPP
#define GEKKOFS_METADATA_DB_HPP

#include <memory>
#include <rocksdb/db.h>
#include <daemon/backend/exceptions.hpp>
#include <tuple>

namespace rdb = rocksdb;

namespace gkfs::metadata {

class MetadataDB {
private:
    std::unique_ptr<rdb::DB> db;
    rdb::Options options;
    rdb::WriteOptions write_opts;
    std::string path;

    static void
    optimize_rocksdb_options(rdb::Options& options);

public:
    static inline void
    throw_rdb_status_excpt(const rdb::Status& s);

    explicit MetadataDB(const std::string& path);

    std::string
    get(const std::string& key) const;

    void
    put(const std::string& key, const std::string& val);

    void
    put_no_exist(const std::string& key, const std::string& val);

    void
    remove(const std::string& key);

    bool
    exists(const std::string& key);

    void
    update(const std::string& old_key, const std::string& new_key,
           const std::string& val);

    void
    increase_size(const std::string& key, size_t size, bool append);

    void
    decrease_size(const std::string& key, size_t size);

    std::vector<std::pair<std::string, bool>>
    get_dirents(const std::string& dir) const;

    std::vector<std::tuple<std::string, bool, size_t, time_t>>
    get_dirents_extended(const std::string& dir) const;

    void
    iterate_all();
};

} // namespace gkfs::metadata

#endif // GEKKOFS_METADATA_DB_HPP
