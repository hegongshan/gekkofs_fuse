/*
  Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany

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

/* C++ includes */
#include <CLI11/CLI11.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <fmt/format.h>
#include <commands.hpp>
#include <reflection.hpp>
#include <serialize.hpp>
#include <binary_buffer.hpp>

/* C includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

using json = nlohmann::json;

// Creates a file in the path, and does a readdir comparing the count size (may be accumulated with previous operations)
struct directory_validate_options {
    bool verbose{};
    std::string pathname;
    ::size_t count;

    REFL_DECL_STRUCT(directory_validate_options, REFL_DECL_MEMBER(bool, verbose),
                     REFL_DECL_MEMBER(std::string, pathname),
                     REFL_DECL_MEMBER(::size_t, count));
};

struct directory_validate_output {
    int retval;
    int errnum;

    REFL_DECL_STRUCT(directory_validate_output, REFL_DECL_MEMBER(int, retval),
                     REFL_DECL_MEMBER(int, errnum));
};

void
to_json(json& record, const directory_validate_output& out) {
    record = serialize(out);
}

void
directory_validate_exec(const directory_validate_options& opts) {

    int fd = ::creat(opts.pathname.c_str(), S_IRWXU);
   
    if(fd == -1) {
        if(1) {
            fmt::print(
                    "directory_validate(pathname=\"{}\", count={}) = {}, errno: {} [{}]\n",
                    opts.pathname, opts.count, fd, errno, ::strerror(errno));
            return;
        }
        json out = directory_validate_output{-2, errno};
        fmt::print("{}\n", out.dump(2));

        return;
    }

    ::close(fd);

    // Do a readdir
    std::string dir = ::dirname((char*)opts.pathname.c_str());
    ::DIR* dirp = ::opendir(dir.c_str());

    if(dirp == NULL) {
        if(opts.verbose) {
            fmt::print("readdir(pathname=\"{}\") = {}, errno: {} [{}]\n",
                       opts.pathname, "NULL", errno, ::strerror(errno));
            return;
        }

    std::cout << "Error create directory" << std::endl;
        json out = directory_validate_output{-3, errno};
        fmt::print("{}\n", out.dump(2));

        return;
    }

    std::vector<struct ::dirent> entries;
    struct ::dirent* entry;

    while((entry = ::readdir(dirp)) != NULL) {
        entries.push_back(*entry);
    }

    if(opts.verbose) {
        fmt::print("readdir(pathname=\"{}\") = [\n{}],\nerrno: {} [{}]\n",
                   opts.pathname, fmt::join(entries, ",\n"), errno,
                   ::strerror(errno));
        return;
    }
    
    errno = 0;
    json out = directory_validate_output{(int) entries.size(), errno};
    fmt::print("{}\n", out.dump(2));
    return;
    
}

void
directory_validate_init(CLI::App& app) {

    // Create the option and subcommand objects
    auto opts = std::make_shared<directory_validate_options>();
    auto* cmd = app.add_subcommand(
            "directory_validate",
            "Create a file and execute a direntry system call and count the number of elements");

    // Add options to cmd, binding them to opts
    cmd->add_flag("-v,--verbose", opts->verbose,
                  "Produce human writeable output");

    cmd->add_option("pathname", opts->pathname, "file name, entries will be checked in last directory")
            ->required()
            ->type_name("");

    cmd->add_option("count", opts->count, "Number of files to check")
            ->required()
            ->type_name("");

    cmd->callback([opts]() { directory_validate_exec(*opts); });
}
