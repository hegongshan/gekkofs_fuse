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

#ifndef GEKKOFS_CHUNK_STORAGE_HPP
#define GEKKOFS_CHUNK_STORAGE_HPP

#include <common/global_defs.hpp>

#include <limits>
#include <string>
#include <memory>
#include <system_error>

/* Forward declarations */
namespace spdlog {
class logger;
}

namespace gkfs {
namespace data {

struct ChunkStat {
    unsigned long chunk_size;
    unsigned long chunk_total;
    unsigned long chunk_free;
};

class ChunkStorageException : public std::system_error {
public:
    ChunkStorageException(const int err_code, const std::string& s)
        : std::system_error(err_code, std::generic_category(), s){};
};

class ChunkStorage {
private:
    std::shared_ptr<spdlog::logger> log_;

    std::string root_path_;
    size_t chunksize_;

    inline std::string
    absolute(const std::string& internal_path) const;

    static inline std::string
    get_chunks_dir(const std::string& file_path);

    static inline std::string
    get_chunk_path(const std::string& file_path, gkfs::rpc::chnk_id_t chunk_id);

    void
    init_chunk_space(const std::string& file_path) const;

public:
    ChunkStorage(std::string& path, size_t chunksize);

    void
    destroy_chunk_space(const std::string& file_path) const;

    ssize_t
    write_chunk(const std::string& file_path, gkfs::rpc::chnk_id_t chunk_id,
                const char* buf, size_t size, off64_t offset) const;

    ssize_t
    read_chunk(const std::string& file_path, gkfs::rpc::chnk_id_t chunk_id,
               char* buf, size_t size, off64_t offset) const;

    void
    trim_chunk_space(const std::string& file_path,
                     gkfs::rpc::chnk_id_t chunk_start);

    void
    truncate_chunk_file(const std::string& file_path,
                        gkfs::rpc::chnk_id_t chunk_id, off_t length);

    ChunkStat
    chunk_stat() const;
};

} // namespace data
} // namespace gkfs

#endif // GEKKOFS_CHUNK_STORAGE_HPP
