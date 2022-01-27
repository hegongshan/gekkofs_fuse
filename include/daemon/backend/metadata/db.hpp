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
/**
 * @brief Class declaration for MetadataDB class which uses RocksDB and provides
 * a single instance within the daemon.
 */
#ifndef GEKKOFS_METADATA_DB_HPP
#define GEKKOFS_METADATA_DB_HPP

#include <memory>
#include <spdlog/spdlog.h>
#include <rocksdb/db.h>
#include <daemon/backend/exceptions.hpp>
#include <tuple>

namespace rdb = rocksdb;

namespace gkfs::metadata {

/**
 * @brief MetadataDB class providing an abstraction layer to the KV store
 * RocksDB.
 */
class MetadataDB {
private:
    std::unique_ptr<rdb::DB> db;  ///< RocksDB instance
    rdb::Options options;         ///< RocksDB configuration
    rdb::WriteOptions write_opts; ///< RocksDB write configuration
    std::string path;             ///< Path to where RocksDB persists its data
    std::shared_ptr<spdlog::logger> log_; ///< MetadataDB internal logger

    /**
     * @brief Sets up specific settings to optimize RocksDB instance to
     * environment on launch.
     * @param options RocksDB configurations
     */
    static void
    optimize_rocksdb_options(rdb::Options& options);

public:
    /**
     * @brief Exception wrapper on Status object.
     * @param s RocksDB status
     * @throws Throws NotFoundException if s.IsNotFound(), general DBException
     * otherwise
     */
    static inline void
    throw_rdb_status_excpt(const rdb::Status& s);

    /**
     * @brief Constructor, called when daemon is started and connects to KV
     * store.
     * @param path Path to where RocksDB persists its data
     */
    explicit MetadataDB(const std::string& path);

    /**
     * @brief Gets the KV store value for a key.
     * @param key KV store key
     * @return KV store value
     * @throws DBException on failure, NotFoundException if entry doesn't exist
     */
    [[nodiscard]] std::string
    get(const std::string& key) const;

    /**
     * @brief Puts an entry into the KV store.
     * @param key KV store key
     * @param val KV store value
     * @throws DBException
     */
    void
    put(const std::string& key, const std::string& val);

    /**
     * @brief Puts an entry into the KV store if it doesn't exist.
     * @param key KV store key
     * @param val KV store value
     * @throws DBException on failure, ExistException if entry already exists
     */
    void
    put_no_exist(const std::string& key, const std::string& val);

    /**
     * @brief Removes an entry from the KV store.
     * @param key KV store key
     * @throws DBException on failure, NotFoundException if entry doesn't exist
     */
    void
    remove(const std::string& key);

    /**
     * @brief Checks for existence of an entry.
     * @param key KV store key
     * @return true if exists
     * @throws DBException on failure, NotFoundException if entry doesn't exist
     */
    bool
    exists(const std::string& key);

    /**
     * Updates a metadata entry atomically and also allows to change keys.
     * @param old_key KV store key to be replaced
     * @param new_key new KV store key
     * @param val KV store value
     * @throws DBException on failure, NotFoundException if entry doesn't exist
     */
    void
    update(const std::string& old_key, const std::string& new_key,
           const std::string& val);

    /**
     * @brief Increases only the size part of the metadata entry via a RocksDB
     * Operand.
     * @param key KV store key
     * @param size new size for entry
     * @param append
     * @throws DBException on failure, NotFoundException if entry doesn't exist
     */
    void
    increase_size(const std::string& key, size_t size, bool append);

    /**
     * @brief Decreases only the size part of the metadata entry via a RocksDB
     * Operand/
     * @param key KV store key
     * @param size new size for entry
     * @throws DBException on failure, NotFoundException if entry doesn't exist
     */
    void
    decrease_size(const std::string& key, size_t size);

    /**
     * @brief Return all file names and modes for the first-level entries of the
     * given directory.
     * @param dir directory prefix string
     * @return vector of pair <std::string name, bool is_dir>,
     *         where name is the name of the entries and is_dir
     *         is true in the case the entry is a directory.
     */
    [[nodiscard]] std::vector<std::pair<std::string, bool>>
    get_dirents(const std::string& dir) const;

    /**
     * @brief Return all file names and modes for the first-level entries of the
     * given directory including their sizes and creation time.
     * @param dir directory prefix string
     * @return vector of pair <std::string name, bool is_dir - size - ctime>,
     *         where name is the name of the entries and is_dir
     *         is true in the case the entry is a directory.
     */
    [[nodiscard]] std::vector<std::tuple<std::string, bool, size_t, time_t>>
    get_dirents_extended(const std::string& dir) const;

    /**
     * @brief Iterate over complete database, note ONLY used for debugging and
     * is therefore unused.
     */
    void
    iterate_all();
};

} // namespace gkfs::metadata

#endif // GEKKOFS_METADATA_DB_HPP
