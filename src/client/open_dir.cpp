/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS' POSIX interface.

  GekkoFS' POSIX interface is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  GekkoFS' POSIX interface is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with GekkoFS' POSIX interface.  If not, see
  <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: LGPL-3.0-or-later
*/

#include <client/open_dir.hpp>
#include <stdexcept>
#include <cstring>

namespace gkfs {
namespace filemap {

DirEntry::DirEntry(const std::string& name, const FileType type)
    : name_(name), type_(type) {}

const std::string&
DirEntry::name() {
    return name_;
}

FileType
DirEntry::type() {
    return type_;
}


OpenDir::OpenDir(const std::string& path)
    : OpenFile(path, 0, FileType::directory) {}


void
OpenDir::add(const std::string& name, const FileType& type) {
    entries.push_back(DirEntry(name, type));
}

const DirEntry&
OpenDir::getdent(unsigned int pos) {
    return entries.at(pos);
}

size_t
OpenDir::size() {
    return entries.size();
}

} // namespace filemap
} // namespace gkfs