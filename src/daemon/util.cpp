/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

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

#include <daemon/util.hpp>
#include <daemon/daemon.hpp>

#include <common/rpc/rpc_util.hpp>

#include <fstream>
#include <iostream>

using namespace std;

namespace gkfs::utils {

void
populate_hosts_file() {
    const auto& hosts_file = GKFS_DATA->hosts_file();
    GKFS_DATA->spdlogger()->debug("{}() Populating hosts file: '{}'", __func__,
                                  hosts_file);
    ofstream lfstream(hosts_file, ios::out | ios::app);
    if(!lfstream) {
        throw runtime_error(fmt::format("Failed to open hosts file '{}': {}",
                                        hosts_file, strerror(errno)));
    }
    lfstream << fmt::format("{} {}", gkfs::rpc::get_my_hostname(true),
                            RPC_DATA->self_addr_str())
             << std::endl;
    if(!lfstream) {
        throw runtime_error(
                fmt::format("Failed to write on hosts file '{}': {}",
                            hosts_file, strerror(errno)));
    }
    lfstream.close();
}

void
destroy_hosts_file() {
    std::remove(GKFS_DATA->hosts_file().c_str());
}

} // namespace gkfs::utils
