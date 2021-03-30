#include <daemon/backend/metadata/db.hpp>
#include <daemon/backend/metadata/merge.hpp>
#include <daemon/backend/exceptions.hpp>

#include <global/metadata.hpp>
#include <global/path_util.hpp>

extern "C" {
#include <sys/stat.h>
}

namespace gkfs::metadata {


/**
 * Called when the daemon is started: Connects to the KV store
 * @param path where KV store data is stored
 */
MetadataDB::MetadataDB(const std::string& path) : path(path) {
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;
    options.merge_operator.reset(new MetadataMergeOperator);
    MetadataDB::optimize_rocksdb_options(options);
    write_opts.disableWAL = !(gkfs::config::rocksdb::use_write_ahead_log);
    rdb::DB* rdb_ptr = nullptr;
    auto s = rocksdb::DB::Open(options, path, &rdb_ptr);
    if(!s.ok()) {
        throw std::runtime_error("Failed to open RocksDB: " + s.ToString());
    }
    this->db.reset(rdb_ptr);
}

/**
 * Exception wrapper on Status object. Throws NotFoundException if
 * s.IsNotFound(), general DBException otherwise
 * @param RocksDB status
 * @throws DBException
 */
void
MetadataDB::throw_rdb_status_excpt(const rdb::Status& s) {
    assert(!s.ok());

    if(s.IsNotFound()) {
        throw NotFoundException(s.ToString());
    } else {
        throw DBException(s.ToString());
    }
}

/**
 * Gets a KV store value for a key
 * @param key
 * @return value
 * @throws DBException on failure, NotFoundException if entry doesn't exist
 */
std::string
MetadataDB::get(const std::string& key) const {
    std::string val;
    auto s = db->Get(rdb::ReadOptions(), key, &val);
    if(!s.ok()) {
        MetadataDB::throw_rdb_status_excpt(s);
    }
    return val;
}

/**
 * Puts an entry into the KV store
 * @param key
 * @param val
 * @throws DBException on failure
 */
void
MetadataDB::put(const std::string& key, const std::string& val) {
    assert(gkfs::path::is_absolute(key));
    assert(key == "/" || !gkfs::path::has_trailing_slash(key));

    auto cop = CreateOperand(val);
    auto s = db->Merge(write_opts, key, cop.serialize());
    if(!s.ok()) {
        MetadataDB::throw_rdb_status_excpt(s);
    }
}

/**
 * Puts an entry into the KV store if it doesn't exist. This function does not
 * use a mutex.
 * @param key
 * @param val
 * @throws DBException on failure, ExistException if entry already exists
 */
void
MetadataDB::put_no_exist(const std::string& key, const std::string& val) {
    if(exists(key))
        throw ExistsException(key);
    put(key, val);
}

/**
 * Removes an entry from the KV store
 * @param key
 * @throws DBException on failure, NotFoundException if entry doesn't exist
 */
void
MetadataDB::remove(const std::string& key) {
    auto s = db->Delete(write_opts, key);
    if(!s.ok()) {
        MetadataDB::throw_rdb_status_excpt(s);
    }
}

/**
 * checks for existence of an entry
 * @param key
 * @return true if exists
 * @throws DBException on failure
 */
bool
MetadataDB::exists(const std::string& key) {
    std::string val;
    auto s = db->Get(rdb::ReadOptions(), key, &val);
    if(!s.ok()) {
        if(s.IsNotFound()) {
            return false;
        } else {
            MetadataDB::throw_rdb_status_excpt(s);
        }
    }
    return true;
}

/**
 * Updates a metadentry atomically and also allows to change keys
 * @param old_key
 * @param new_key
 * @param val
 * @throws DBException on failure, NotFoundException if entry doesn't exist
 */
void
MetadataDB::update(const std::string& old_key, const std::string& new_key,
                   const std::string& val) {
    // TODO use rdb::Put() method
    rdb::WriteBatch batch;
    batch.Delete(old_key);
    batch.Put(new_key, val);
    auto s = db->Write(write_opts, &batch);
    if(!s.ok()) {
        MetadataDB::throw_rdb_status_excpt(s);
    }
}

/**
 * Increases only the size part of the metadentry via a RocksDB Operand
 * Operation. E.g., called before a write() call
 * @param key
 * @param size
 * @param append
 * @throws DBException on failure
 */
void
MetadataDB::increase_size(const std::string& key, size_t size, bool append) {
    auto uop = IncreaseSizeOperand(size, append);
    auto s = db->Merge(write_opts, key, uop.serialize());
    if(!s.ok()) {
        MetadataDB::throw_rdb_status_excpt(s);
    }
}

/**
 * Decreases only the size part of the metadentry via a RocksDB Operand
 * Operation E.g., called before a truncate() call
 * @param key
 * @param size
 * @throws DBException on failure
 */
void
MetadataDB::decrease_size(const std::string& key, size_t size) {
    auto uop = DecreaseSizeOperand(size);
    auto s = db->Merge(write_opts, key, uop.serialize());
    if(!s.ok()) {
        MetadataDB::throw_rdb_status_excpt(s);
    }
}

/**
 * Return all the first-level entries of the directory @dir
 *
 * @return vector of pair <std::string name, bool is_dir>,
 *         where name is the name of the entries and is_dir
 *         is true in the case the entry is a directory.
 */
std::vector<std::pair<std::string, bool>>
MetadataDB::get_dirents(const std::string& dir) const {
    auto root_path = dir;
    assert(gkfs::path::is_absolute(root_path));
    // add trailing slash if missing
    if(!gkfs::path::has_trailing_slash(root_path) && root_path.size() != 1) {
        // add trailing slash only if missing and is not the root_folder "/"
        root_path.push_back('/');
    }

    rocksdb::ReadOptions ropts;
    auto it = db->NewIterator(ropts);

    std::vector<std::pair<std::string, bool>> entries;

    for(it->Seek(root_path); it->Valid() && it->key().starts_with(root_path);
        it->Next()) {

        if(it->key().size() == root_path.size()) {
            // we skip this path cause it is exactly the root_path
            continue;
        }

        /***** Get File name *****/
        auto name = it->key().ToString();
        if(name.find_first_of('/', root_path.size()) != std::string::npos) {
            // skip stuff deeper then one level depth
            continue;
        }
        // remove prefix
        name = name.substr(root_path.size());

        // relative path of directory entries must not be empty
        assert(!name.empty());

        Metadata md(it->value().ToString());
        auto is_dir = S_ISDIR(md.mode());

        entries.emplace_back(std::move(name), is_dir);
    }
    assert(it->status().ok());
    return entries;
}

/**
 * Return all the first-level entries of the directory @dir
 *
 * @return vector of pair <std::string name, bool is_dir - size - ctime>,
 *         where name is the name of the entries and is_dir
 *         is true in the case the entry is a directory.
 */
std::vector<std::tuple<std::string, bool, size_t, time_t>>
MetadataDB::get_dirents_extended(const std::string& dir) const {
    auto root_path = dir;
    assert(gkfs::path::is_absolute(root_path));
    // add trailing slash if missing
    if(!gkfs::path::has_trailing_slash(root_path) && root_path.size() != 1) {
        // add trailing slash only if missing and is not the root_folder "/"
        root_path.push_back('/');
    }

    rocksdb::ReadOptions ropts;
    auto it = db->NewIterator(ropts);

    std::vector<std::tuple<std::string, bool, size_t, time_t>> entries;

    for(it->Seek(root_path); it->Valid() && it->key().starts_with(root_path);
        it->Next()) {

        if(it->key().size() == root_path.size()) {
            // we skip this path cause it is exactly the root_path
            continue;
        }

        /***** Get File name *****/
        auto name = it->key().ToString();
        if(name.find_first_of('/', root_path.size()) != std::string::npos) {
            // skip stuff deeper then one level depth
            continue;
        }
        // remove prefix
        name = name.substr(root_path.size());

        // relative path of directory entries must not be empty
        assert(!name.empty());

        Metadata md(it->value().ToString());
        auto is_dir = S_ISDIR(md.mode());

        entries.emplace_back(std::forward_as_tuple(std::move(name), is_dir,
                                                   md.size(), md.ctime()));
    }
    assert(it->status().ok());
    return entries;
}


/**
 * Code example for iterating all entries in KV store. This is for debug only as
 * it is too expensive
 */
void
MetadataDB::iterate_all() {
    std::string key;
    std::string val;
    // Do RangeScan on parent inode
    auto iter = db->NewIterator(rdb::ReadOptions());
    for(iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        key = iter->key().ToString();
        val = iter->value().ToString();
    }
}

/**
 * Called when RocksDB connection is established.
 * Used for setting KV store settings
 * see here: https://github.com/facebook/rocksdb/wiki/RocksDB-Tuning-Guide
 * @param options
 */
void
MetadataDB::optimize_rocksdb_options(rdb::Options& options) {
    options.max_successive_merges = 128;
}

} // namespace gkfs::metadata
