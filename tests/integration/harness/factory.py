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

import os, re, hashlib
from harness.logger import logger
from harness.gkfs import FwdDaemon, FwdClient

class FwdDaemonCreator:
    """
    Factory that allows tests to create forwarding daemons in a workspace.
    """

    def __init__(self, interface, workspace):
        self._interface = interface
        self._workspace = workspace

    def create(self):
        """
        Create a forwarding daemon in the tests workspace.

        Returns
        -------
        The `FwdDaemon` object to interact with the daemon.
        """

        daemon = FwdDaemon(self._interface, self._workspace)
        daemon.run()

        return daemon

class FwdClientCreator:
    """
    Factory that allows tests to create forwarding daemons in a workspace.
    """

    def __init__(self, workspace):
        self._workspace = workspace

    def create(self, identifier):
        """
        Create a forwarding client in the tests workspace.

        Returns
        -------
        The `FwdClient` object to interact with the daemon.
        """

        return FwdClient(self._workspace, identifier)
