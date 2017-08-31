//
// Created by evie on 8/31/17.
//

#include "daemon/adafs_daemon.hpp"
#include "db/db_util.hpp"

/**
 * Initializes the Argobots environment
 * @return
 */
bool init_argobots() {
    ADAFS_DATA->spdlogger()->info("Initializing Argobots ...");

    // We need no arguments to init
    auto argo_err = ABT_init(0, nullptr);
    if (argo_err != 0) {
        ADAFS_DATA->spdlogger()->error("ABT_init() Failed to init Argobots (client)");
        return false;
    }
    // Set primary execution stream to idle without polling. Normally xstreams cannot sleep. This is what ABT_snoozer does
    argo_err = ABT_snoozer_xstream_self_set();
    if (argo_err != 0) {
        ADAFS_DATA->spdlogger()->error("ABT_snoozer_xstream_self_set()  (client)");
        return false;
    }
    ADAFS_DATA->spdlogger()->info("Success.");
    return true;
}

/**
 * Shuts down Argobots
 */
void destroy_argobots() {
    ADAFS_DATA->spdlogger()->info("About to shut Argobots down"s);
    auto ret = ABT_finalize();
    if (ret == ABT_SUCCESS) {
        ADAFS_DATA->spdlogger()->info("Argobots successfully shutdown.");
    } else {
        ADAFS_DATA->spdlogger()->error("Argobots shutdown FAILED with err code {}", ret);
    }
}

void init_environment() {
    // Initialize rocksdb
    auto err = init_rocksdb();
    assert(err);
    err = init_argobots();
    assert(err);

    destroy_argobots();

}
