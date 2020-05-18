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



#TESTING LSEEK


#SEEK_SET: int
#SEEK_CUR: int
#SEEK_END: int

# 1. LSeek on empty file
# 2. LSeek on non-empty file



#@pytest.mark.xfail(reason="invalid errno returned on success")
def test_lseek(gkfs_daemon, gkfs_client):
    """Test several statx commands"""
    topdir = gkfs_daemon.mountdir / "top"
    longer = Path(topdir.parent, topdir.name + "_plus")
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

    # LSeek on empty file

    ret = gkfs_client.lseek(file_a, 0, os.SEEK_SET)

    assert ret.retval == 0

    ret = gkfs_client.lseek(file_a, -1, os.SEEK_SET)
    assert ret.retval == -1
    assert ret.errno == 115 #FIXME: Should be 22

    # We can go further an empty file

    ret = gkfs_client.lseek(file_a, 5, os.SEEK_SET)
    assert ret.retval == 5
    assert ret.errno == 115 #FIXME: Should be 0

    # Size needs to be 0 
    ret = gkfs_client.statx(0, file_a, 0, 0)

    assert ret.retval == 0
    assert (stat.S_ISDIR(ret.statbuf.stx_mode)==0)
    assert (ret.statbuf.stx_size == 0)


    # next commands write at the end of the file (pos0), as the filedescriptor is not shared
    buf = b'42'
    ret = gkfs_client.write(file_a, buf, 2)

    assert ret.retval == 2

    # Size should be 2
    ret = gkfs_client.statx(0, file_a, 0, 0)

    assert ret.retval == 0
    assert (ret.statbuf.stx_size == 2)
    
    ret = gkfs_client.lseek(file_a, 0, os.SEEK_END)
    assert ret.retval == 2                      #FAILS
    assert ret.errno == 115 #FIXME: Should be 0 

    ret = gkfs_client.lseek(file_a, -2, os.SEEK_END)
    assert ret.retval == 0                     #FAILS
    assert ret.errno == 115 #FIXME: Should be 0 

    ret = gkfs_client.lseek(file_a, -3, os.SEEK_END)
    assert ret.retval == -1                     #FAILS
    assert ret.errno == 22 





  
    return


