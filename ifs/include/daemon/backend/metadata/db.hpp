#ifndef IFS_METADATA_DB_HPP
#define IFS_METADATA_DB_HPP

#include <memory>
#include "rocksdb/db.h"
#include "daemon/backend/exceptions.hpp"

namespace rdb = rocksdb;

class MetadataDB {
    private:
        std::unique_ptr<rdb::DB> db;
        rdb::Options options;
        rdb::WriteOptions write_opts;
        std::string path;
        static void optimize_rocksdb_options(rdb::Options& options);

    public:
        static inline void throw_rdb_status_excpt(const rdb::Status& s);

        MetadataDB(const std::string& path);

        std::string get(const std::string& key) const;
        bool put(const std::string& key, const std::string& val);
        void remove(const std::string& key);
        bool exists(const std::string& key);
        bool update(const std::string& old_key, const std::string& new_key, const std::string& val);
        bool update_size(const std::string& key, size_t size, off64_t offset, bool append);
        void iterate_all();
};

#endif //IFS_METADATA_DB_HPP
