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

struct symlink_options {
    bool verbose{};
    std::string pathname;
    std::string linkpath;

    REFL_DECL_STRUCT(symlink_options,
        REFL_DECL_MEMBER(bool, verbose),
        REFL_DECL_MEMBER(std::string, pathname),
        REFL_DECL_MEMBER(std::string, linkpath)
    );
};

struct symlink_output {
    int retval;
    int errnum;

    REFL_DECL_STRUCT(symlink_output,
        REFL_DECL_MEMBER(int, retval),
        REFL_DECL_MEMBER(int, errnum)
    );
};

void
to_json(json& record,
        const symlink_output& out) {
    record = serialize(out);
}

void
symlink_exec(const symlink_options& opts) {

    auto rv = ::symlink(opts.pathname.c_str(), opts.linkpath.c_str());

    if(opts.verbose) {
        fmt::print("symlink(pathname=\"{}\", linkpath=\"{}\") = {}, errno: {} [{}]\n",
                   opts.pathname, opts.linkpath, rv, errno, ::strerror(errno));
        return;
    }

    json out = symlink_output{rv, errno};
    fmt::print("{}\n", out.dump(2));
}

void
symlink_init(CLI::App& app) {

    // Create the option and subcommand objects
    auto opts = std::make_shared<symlink_options>();
    auto* cmd = app.add_subcommand(
            "symlink",
            "Execute the symlink() system call");

    // Add options to cmd, binding them to opts
    cmd->add_flag(
            "-v,--verbose",
            opts->verbose,
            "Produce human readable output"
        );

    cmd->add_option(
            "pathname",
            opts->pathname,
            "Directory name"
        )
        ->required()
        ->type_name("");

    cmd->add_option(
            "linkpath",
            opts->linkpath,
            "link path name"
        )
        ->required()
        ->type_name("");

    cmd->callback([opts]() {
        symlink_exec(*opts);
    });
}
