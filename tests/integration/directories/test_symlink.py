################################################################################
# Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# SPDX-License-Identifier: MIT                                                 #
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
def test_symlink(gkfs_daemon, gkfs_client):
    """Testing different path resolution capabilities: symlinks"""

    mountdir = gkfs_daemon.mountdir
    extdir = "/tmp/ext.tmp"
    ext_linkdir = "/tmp/link.tmp"
    nodir = "/tmp/notexistent"
    intdir = mountdir / "int"

    # Just clean if it exists, due to a failed test

    ret = gkfs_client.rmdir(extdir)
    try:
        os.unlink(ext_linkdir) # it is external
    except Exception as e:
        pass

    ret = gkfs_client.mkdir(
        extdir,
        stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 0

    ret = gkfs_client.mkdir(
        intdir,
        stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 0

    #Test external symlink

    ret = gkfs_client.symlink(extdir, ext_linkdir)
    assert ret.retval == 0

    ret = gkfs_client.getcwd_validate(str(extdir) + "/../link.tmp")
    assert ret.path == str(extdir)
    assert ret.retval == 0

    ret = gkfs_client.getcwd_validate(str(intdir) + "/../../../../../../../../../../../../../../../../../../../.."+str(extdir) + "/../link.tmp/../link.tmp/../../../../../../../../../../../../../../../../../../../../.."  + str(intdir))
    assert ret.path == str(intdir)
    assert ret.retval == 0

    # Teardown

    # Clean external link
    os.unlink(ext_linkdir)

    ret = gkfs_client.rmdir(extdir)
    assert ret.retval == 0

    #  Clean internal dir
    ret = gkfs_client.rmdir(intdir)
    assert ret.retval == 0

    return
