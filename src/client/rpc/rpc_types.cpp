#include <hermes.hpp>
#include <client/rpc/rpc_types.hpp>

//==============================================================================
// register request types so that they can be used by users and the engine
//
void
hermes::detail::register_user_request_types() {
    (void) registered_requests().add<gkfs::rpc::fs_config>();
    (void) registered_requests().add<gkfs::rpc::create>();
    (void) registered_requests().add<gkfs::rpc::stat>();
    (void) registered_requests().add<gkfs::rpc::remove_metadata>();
    (void) registered_requests().add<gkfs::rpc::decr_size>();
    (void) registered_requests().add<gkfs::rpc::update_metadentry>();
    (void) registered_requests().add<gkfs::rpc::get_metadentry_size>();
    (void) registered_requests().add<gkfs::rpc::update_metadentry_size>();

#ifdef HAS_SYMLINKS
    (void) registered_requests().add<gkfs::rpc::mk_symlink>();
#endif // HAS_SYMLINKS
    (void) registered_requests().add<gkfs::rpc::remove_data>();
    (void) registered_requests().add<gkfs::rpc::write_data>();
    (void) registered_requests().add<gkfs::rpc::read_data>();
    (void) registered_requests().add<gkfs::rpc::trunc_data>();
    (void) registered_requests().add<gkfs::rpc::get_dirents>();
    (void) registered_requests().add<gkfs::rpc::chunk_stat>();
    (void) registered_requests().add<gkfs::rpc::get_dirents_extended>();
}
