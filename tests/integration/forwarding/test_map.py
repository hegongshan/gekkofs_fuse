################################################################################
#  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain           #
#  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany         #
#                                                                              #
#  This software was partially supported by the                                #
#  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).   #
#                                                                              #
#  This software was partially supported by the                                #
#  ADA-FS project under the SPPEXA project funded by the DFG.                  #
#                                                                              #
#  SPDX-License-Identifier: MIT                                                #
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


def test_read(gkfs_daemon, gkfs_client):

    file = gkfs_daemon.mountdir / "file"

    # create a file in gekkofs
    ret = gkfs_client.open(file,
                           os.O_CREAT | os.O_WRONLY,
                           stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 10000
    assert ret.errno == 115 #FIXME: Should be 0!

    # write a buffer we know
    buf = b'42'
    ret = gkfs_client.write(file, buf, len(buf))

    assert ret.retval == len(buf) # Return the number of written bytes
    assert ret.errno == 115 #FIXME: Should be 0!

    # open the file to read
    ret = gkfs_client.open(file,
                           os.O_RDONLY,
                           stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 10000
    assert ret.errno == 115 #FIXME: Should be 0!

    # read the file
    ret = gkfs_client.read(file, len(buf))

    assert ret.buf == buf
    assert ret.retval == len(buf) # Return the number of read bytes
    assert ret.errno == 115 #FIXME: Should be 0!
