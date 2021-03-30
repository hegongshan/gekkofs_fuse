import pytest
from harness.logger import logger

file01 = 'file01'

@pytest.mark.skip(reason="shell tests seem to hang clients at times")
def test_cp(gkfs_daemon, gkfs_shell, file_factory):
    """Copy a file into gkfs using the shell"""

    logger.info("creating input file")
    lf01 = file_factory.create(file01, size=4.0, unit='MB')

    logger.info("copying into gkfs")
    cmd = gkfs_shell.cp(lf01.pathname, gkfs_daemon.mountdir)
    assert cmd.exit_code == 0
    assert cmd.stdout.decode() == ''
    assert cmd.stderr.decode() == ''
