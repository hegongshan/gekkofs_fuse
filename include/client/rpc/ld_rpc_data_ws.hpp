/*
  Copyright 2018-2019, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2019, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/


#ifndef GEKKOFS_PRELOAD_C_DATA_WS_HPP
#define GEKKOFS_PRELOAD_C_DATA_WS_HPP


namespace rpc_send {

    struct ChunkStat {
        unsigned long chunk_size;
        unsigned long chunk_total;
        unsigned long chunk_free;
    };

    ssize_t write(const std::string& path, const void* buf, bool append_flag, off64_t in_offset,
                  size_t write_size, int64_t updated_metadentry_size);

    ssize_t read(const std::string& path, void* buf, off64_t offset, size_t read_size);

    int trunc_data(const std::string& path, size_t current_size, size_t new_size);

    ChunkStat chunk_stat();

}


#endif //GEKKOFS_PRELOAD_C_DATA_WS_HPP
