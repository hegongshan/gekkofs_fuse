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
import sh
import sys
import pytest
from harness.logger import logger

nonexisting = "nonexisting"


def test_open_error(gkfs_daemon, gkfs_client):

    file = gkfs_daemon.mountdir / "file"
    file2 = gkfs_daemon.mountdir / "file2"
    file3 = gkfs_daemon.mountdir / "file3"
    
    flags = [os.O_PATH, os.O_APPEND, os.O_CREAT | os.O_DIRECTORY]
    # create a file in gekkofs

    for flag in flags:
        ret = gkfs_client.open(file,
                            flag,
                            stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

        assert ret.retval == -1
        assert ret.errno == errno.ENOTSUP


    # Create file and recreate
    ret = gkfs_client.open(file, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    assert ret.retval == 10000
    
    ret = gkfs_client.open(file, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    assert ret.retval == -1
    assert ret.errno == errno.EEXIST


    # Create file and recreate
    ret = gkfs_client.open(file2, os.O_CREAT | os.O_WRONLY)
    assert ret.retval == 10000
    # Undefined in man 
    ret = gkfs_client.open(file2, os.O_CREAT | os.O_WRONLY)
    assert ret.retval == 10000
   

    # Open unexistent file
    ret =  ret = gkfs_client.open(file3, os.O_WRONLY)
    assert ret.retval == -1
    assert ret.errno == errno.ENOENT

    