/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

/* C++ includes */
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <fmt/format.h>
#include <commands.hpp>
#include <reflection.hpp>
#include <serialize.hpp>

/* C includes */
#include <unistd.h>

using json = nlohmann::json;

struct getcwd_validate_options {
    bool verbose{};
    std::string pathname;

    REFL_DECL_STRUCT(getcwd_validate_options, REFL_DECL_MEMBER(bool, verbose),
                     REFL_DECL_MEMBER(std::string, pathname));
};

struct getcwd_validate_output {
    int retval;
    std::string path;
    int errnum;

    REFL_DECL_STRUCT(getcwd_validate_output, REFL_DECL_MEMBER(int, retval),
                     REFL_DECL_MEMBER(std::string, path),
                     REFL_DECL_MEMBER(int, errnum));
};

void
to_json(json& record, const getcwd_validate_output& out) {
    record = serialize(out);
}

void
getcwd_validate_exec(const getcwd_validate_options& opts) {


    char path[1024];
    auto rv = ::chdir(opts.pathname.c_str());
    if(rv == 0) {

        char* cwd = ::getcwd(path, sizeof(path));

        if(cwd == nullptr) {
            rv = -1;
        }

        if(path == opts.pathname)
            if(opts.verbose) {
                fmt::print(
                        "getcwd_validate(pathname=\"{}\") = {}, errno: {} [{}]\n",
                        opts.pathname, cwd, errno, ::strerror(errno));
                return;
            }
    }

    json out = getcwd_validate_output{rv, path, errno};
    fmt::print("{}\n", out.dump(2));
}

void
getcwd_validate_init(CLI::App& app) {

    // Create the option and subcommand objects
    auto opts = std::make_shared<getcwd_validate_options>();
    auto* cmd = app.add_subcommand("getcwd_validate",
                                   "Execute the getcwd_validate() system call");

    // Add options to cmd, binding them to opts
    cmd->add_flag("-v,--verbose", opts->verbose,
                  "Produce human readable output");

    cmd->add_option("pathname", opts->pathname, "Directory name")
            ->required()
            ->type_name("");

    cmd->callback([opts]() { getcwd_validate_exec(*opts); });
}
