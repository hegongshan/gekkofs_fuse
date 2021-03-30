#ifndef GEKKOFS_RPC_DISTRIBUTOR_HPP
#define GEKKOFS_RPC_DISTRIBUTOR_HPP

#include <vector>
#include <string>
#include <numeric>

namespace gkfs::rpc {

using chunkid_t = unsigned int;
using host_t = unsigned int;

class Distributor {
public:
    virtual host_t
    localhost() const = 0;

    virtual host_t
    locate_data(const std::string& path, const chunkid_t& chnk_id) const = 0;

    virtual host_t
    locate_file_metadata(const std::string& path) const = 0;

    virtual std::vector<host_t>
    locate_directory_metadata(const std::string& path) const = 0;
};


class SimpleHashDistributor : public Distributor {
private:
    host_t localhost_;
    unsigned int hosts_size_;
    std::vector<host_t> all_hosts_;
    std::hash<std::string> str_hash;

public:
    SimpleHashDistributor(host_t localhost, unsigned int hosts_size);

    host_t
    localhost() const override;

    host_t
    locate_data(const std::string& path,
                const chunkid_t& chnk_id) const override;

    host_t
    locate_file_metadata(const std::string& path) const override;

    std::vector<host_t>
    locate_directory_metadata(const std::string& path) const override;
};

class LocalOnlyDistributor : public Distributor {
private:
    host_t localhost_;

public:
    explicit LocalOnlyDistributor(host_t localhost);

    host_t
    localhost() const override;

    host_t
    locate_data(const std::string& path,
                const chunkid_t& chnk_id) const override;

    host_t
    locate_file_metadata(const std::string& path) const override;

    std::vector<host_t>
    locate_directory_metadata(const std::string& path) const override;
};

class ForwarderDistributor : public Distributor {
private:
    host_t fwd_host_;
    unsigned int hosts_size_;
    std::vector<host_t> all_hosts_;
    std::hash<std::string> str_hash;

public:
    ForwarderDistributor(host_t fwhost, unsigned int hosts_size);

    host_t
    localhost() const override final;

    host_t
    locate_data(const std::string& path,
                const chunkid_t& chnk_id) const override final;

    host_t
    locate_file_metadata(const std::string& path) const override;

    std::vector<host_t>
    locate_directory_metadata(const std::string& path) const override;
};

} // namespace gkfs::rpc

#endif // GEKKOFS_RPC_LOCATOR_HPP
