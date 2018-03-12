
#ifndef LFS_FS_DATA_H
#define LFS_FS_DATA_H

#include <daemon/adafs_daemon.hpp>


class FsData {

private:
    FsData() {}

    // Caching
    std::unordered_map<std::string, std::string> hashmap_;
    std::hash<std::string> hashf_;

    // inodes
    ino_t inode_count_;

    // Later the blocksize will likely be coupled to the chunks to allow individually big chunk sizes.
    blksize_t blocksize_;

    //logger
    std::shared_ptr<spdlog::logger> spdlogger_;

    // paths
    std::string rootdir_;
    std::string mountdir_;
    std::string metadir_;
    std::string inode_path_;
    std::string dentry_path_;
    std::string chunk_path_;
    std::string mgmt_path_;

    // hosts_
    std::string hosts_raw_; // raw hosts string, given when daemon is started. Used to give it to fs client
    std::map<uint64_t, std::string> hosts_;
    uint64_t host_id_; // my host number
    size_t host_size_;
    std::string rpc_port_;

    // rocksdb
    std::shared_ptr<rocksdb::DB> rdb_;
    std::shared_ptr<rocksdb::DB> rdb_crt_; // additional db instance (currently not used)
    std::shared_ptr<rocksdb::OptimisticTransactionDB> txn_rdb_;
    rocksdb::Options rdb_options_;
    rocksdb::OptimisticTransactionOptions txn_rdb_options_; // needed for snapshots
    rocksdb::WriteOptions rdb_write_options_;
    std::string rdb_path_;

    // configurable metadata
    bool atime_state_;
    bool mtime_state_;
    bool ctime_state_;
    bool uid_state_;
    bool gid_state_;
    bool inode_no_state_;
    bool link_cnt_state_;
    bool blocks_state_;

public:

    // mutex has a deleted method to assign an existing mutex. As such it cannot use getter or setters
    std::mutex inode_mutex;


    static FsData* getInstance() {
        static FsData instance;
        return &instance;
    }

    FsData(FsData const&) = delete;

    void operator=(FsData const&) = delete;

    // Utility member functions

    ino_t raise_inode_count(ino_t count);

    bool is_local_op(size_t recipient);

    size_t hash_path(const std::string& path);

    // getter/setter

    const std::unordered_map<std::string, std::string>& hashmap() const;

    void hashmap(const std::unordered_map<std::string, std::string>& hashmap_);

    const std::hash<std::string>& hashf() const;

    void hashf(const std::hash<std::string>& hashf_);

    ino_t inode_count() const;

    void inode_count(ino_t inode_count);

    blksize_t blocksize() const;

    void blocksize(blksize_t blocksize_);

    const std::shared_ptr<spdlog::logger>& spdlogger() const;

    void spdlogger(const std::shared_ptr<spdlog::logger>& spdlogger_);

    const std::string& rootdir() const;

    void rootdir(const std::string& rootdir_);

    const std::string& mountdir() const;

    void mountdir(const std::string& mountdir_);

    const std::string& metadir() const;

    void metadir(const std::string& metadir_);

    const std::string& inode_path() const;

    void inode_path(const std::string& inode_path_);

    const std::string& dentry_path() const;

    void dentry_path(const std::string& dentry_path_);

    const std::string& chunk_path() const;

    void chunk_path(const std::string& chunk_path_);

    const std::string& mgmt_path() const;

    void mgmt_path(const std::string& mgmt_path_);

    const std::shared_ptr<rocksdb::DB>& rdb() const;

    void rdb(const std::shared_ptr<rocksdb::DB>& rdb);

    const rocksdb::Options& rdb_options() const;

    void rdb_options(const rocksdb::Options& rdb_options);

    const std::string& rdb_path() const;

    void rdb_path(const std::string& rdb_path);

    const std::shared_ptr<rocksdb::OptimisticTransactionDB>& txn_rdb() const;

    void txn_rdb(const std::shared_ptr<rocksdb::OptimisticTransactionDB>& tx_rdb);

    const std::shared_ptr<rocksdb::DB>& rdb_crt() const;

    void rdb_crt(const std::shared_ptr<rocksdb::DB>& rdb_crt);

    const rocksdb::OptimisticTransactionOptions& txn_rdb_options() const;

    void txn_rdb_options(const rocksdb::OptimisticTransactionOptions& tx_rdb_options);

    const rocksdb::WriteOptions& rdb_write_options() const;

    void rdb_write_options(const rocksdb::WriteOptions& rdb_write_options);

    const std::string& hosts_raw() const;

    void hosts_raw(const std::string& hosts_raw);

    const std::map<uint64_t, std::string>& hosts() const;

    void hosts(const std::map<uint64_t, std::string>& hosts);

    const uint64_t& host_id() const;

    void host_id(const uint64_t& host_id);

    size_t host_size() const;

    void host_size(size_t host_size);

    std::string rpc_port() const;

    void rpc_port(std::string rpc_port);

    bool atime_state() const;

    void atime_state(bool atime_state);

    bool mtime_state() const;

    void mtime_state(bool mtime_state);

    bool ctime_state() const;

    void ctime_state(bool ctime_state);

    bool uid_state() const;

    void uid_state(bool uid_state);

    bool gid_state() const;

    void gid_state(bool gid_state);

    bool inode_no_state() const;

    void inode_no_state(bool inode_no_state);

    bool link_cnt_state() const;

    void link_cnt_state(bool link_cnt_state);

    bool blocks_state() const;

    void blocks_state(bool blocks_state);

};


#endif //LFS_FS_DATA_H
