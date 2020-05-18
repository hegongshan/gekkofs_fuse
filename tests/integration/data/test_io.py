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




#@pytest.mark.xfail(reason="invalid errno returned on success")
def test_io(gkfs_daemon, gkfs_client):
    """Test several statx commands"""
    topdir = gkfs_daemon.mountdir / "top"
    file_a = topdir / "file_a"

    # create topdir
    ret = gkfs_client.mkdir(
            topdir,
            stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 0
    assert ret.errno == 115 #FIXME: Should be 0!

    # test statx on existing dir
    ret = gkfs_client.statx(0, topdir, 0, 0)

    assert ret.retval == 0
    assert ret.errno == 115 #FIXME: Should be 0!
    assert stat.S_ISDIR(ret.statbuf.stx_mode)

    ret = gkfs_client.open(file_a,
                   os.O_CREAT,
                   stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval != -1


    # test statx on existing file
    ret = gkfs_client.statx(0, file_a, 0, 0)

    assert ret.retval == 0
    assert (stat.S_ISDIR(ret.statbuf.stx_mode)==0)
    assert (ret.statbuf.stx_size == 0)


    # Phase 1
    # from 1 to n data
    # Generate writes
    # Read data
    # Compare buffer
    # delete data

    # from 1 to 2M +1
    
    for i in range (1, 512, 64):
        buf = b''
        for k in range (0,i):
            value = str(k%10)
            buf += bytes(value, sys.stdout.encoding)
        
        ret = gkfs_client.write(file_a, buf, i)

        assert ret.retval == i
        ret = gkfs_client.statx(0, file_a, 0, 0)

        assert ret.retval == 0
        assert (ret.statbuf.stx_size == i)

        ret = gkfs_client.read(file_a, i)
        assert ret.retval== i
        assert ret.buf == buf


    for i in range (128192, 2097153, 4096*9):
        ret = gkfs_client.write_read(file_a, i)
        assert ret.retval == 1


    return


