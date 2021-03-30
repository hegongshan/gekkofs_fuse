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
def test_pathresolution(gkfs_daemon, gkfs_client):
    """Testing different path resolution capabilities"""

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


    # test stat on existing dir
    ret = gkfs_client.stat(nodir)

    assert ret.retval == -1
    assert ret.errno == errno.ENOENT

    ret = gkfs_client.chdir(nodir)
    assert ret.retval == -1
    assert ret.errno == errno.ENOENT


    # Chdir to external dir

    ret = gkfs_client.chdir(extdir)
    assert ret.retval == 0

    ret = gkfs_client.getcwd_validate(str(intdir)+"../../../../../../../../../../../../../../../../../../.."+str(intdir))
    assert ret.path == str(intdir)
    assert ret.retval == 0

    # test path resolution: from inside to outside
    ret = gkfs_client.getcwd_validate("../../../../../../../../../../../.." + str(extdir))
    assert ret.path == str(extdir)
    assert ret.retval == 0

    # test complex path resolution
    ret = gkfs_client.getcwd_validate("../../../../../../../../../../../.." + str(extdir) + "/../../../../../../../../../../../.." + str(intdir))
    assert ret.path == str(intdir)
    assert ret.retval == 0

    # Teardown

    ret = gkfs_client.rmdir(extdir)
    assert ret.retval == 0

   #  Clean internal dir
    ret = gkfs_client.rmdir(intdir)
    assert ret.retval == 0

    return
