################################################################################
# Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
################################################################################

import harness
from pathlib import Path
import errno
import stat
import os
import ctypes
import sys
import pytest
from harness.logger import logger

nonexisting = "nonexisting"


def test_rename(gkfs_daemon, gkfs_client):

    file = gkfs_daemon.mountdir / "file"
    file2 = gkfs_daemon.mountdir / "file2"

    # create a file in gekkofs
    ret = gkfs_client.open(file,
                           os.O_CREAT | os.O_WRONLY,
                           stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 10000

    # write a buffer we know
    buf = b'42'
    ret = gkfs_client.write(file, buf, len(buf))

    assert ret.retval == len(buf) # Return the number of written bytes

    ret = gkfs_client.stat(file)
    assert ret.retval != 1

    ret = gkfs_client.stat(file2)
    assert ret.retval == -1

    ret = gkfs_client.rename(file, file2)

    assert ret.retval == 0

    ret = gkfs_client.stat(file)
    assert ret.retval == -1

    ret = gkfs_client.stat(file2)
    assert ret.retval != 1

    # File is renamed, and innacesible

    # Read contents of file2
    ret = gkfs_client.open(file2, os.O_RDONLY)
    assert ret.retval == 10000

    ret = gkfs_client.read(file2, len(buf))
    assert ret.retval == len(buf)
    assert ret.buf == buf

    

def test_rename_inverse(gkfs_daemon, gkfs_client):

    file3 = gkfs_daemon.mountdir / "file3"
    file4 = gkfs_daemon.mountdir / "file4"

    # create a file in gekkofs
    ret = gkfs_client.open(file3,
                           os.O_CREAT | os.O_WRONLY,
                           stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 10000

 

    ret = gkfs_client.stat(file3)
    assert ret.retval != 1

    ret = gkfs_client.stat(file4)
    assert ret.retval == -1

    ret = gkfs_client.rename(file3, file4)

    assert ret.retval == 0

    ret = gkfs_client.stat(file3)
    assert ret.retval == -1

    ret = gkfs_client.stat(file4)
    assert ret.retval != 1

    # File is renamed, and innacesible

   # write a buffer we know
    buf = b'42'
    ret = gkfs_client.write(file4, buf, len(buf))

    assert ret.retval == len(buf) # Return the number of written bytes

    # Read contents of file2
    ret = gkfs_client.open(file4, os.O_RDONLY)
    assert ret.retval == 10000

    ret = gkfs_client.read(file4, len(buf))
    assert ret.retval == len(buf)
    assert ret.buf == buf

    # It should work but the data should be on file 2 really

