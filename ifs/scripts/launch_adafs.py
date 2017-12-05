#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

import os

from util import util

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

PRETEND = False


def init_system(daemon_path, rootdir, mountdir, nodelist):
    if not os.path.exists(daemon_path) or not os.path.isfile(daemon_path):
        print '[ERR] Daemon executable not found or not a file'
    nodefile = False
    if os.path.exists(nodelist):
        nodefile = True
    else:
        nodelist.replace(',', ' ')
    cmd_str = 'parallel-ssh -i -H "%s" "nohup %s -r %s -m %s --hosts %s >> /tmp/adafs_startup.log 2>&1 &"' \
              % (nodelist, daemon_path, rootdir, mountdir, nodelist)
    print 'Running: %s' % cmd_str
    if PRETEND is True:
        print cmd_str
    else:
        util.exec_shell(cmd_str)
    cmd_chk_str = 'parallel-ssh -i -H "%s" "cat /tmp/adafs_startup.log"' % nodelist
    print 'Running: %s' % cmd_chk_str
    if PRETEND is True:
        print cmd_chk_str
    else:
        util.exec_shell(cmd_chk_str)


if __name__ == "__main__":
    # Init parser
    parser = argparse.ArgumentParser(description='This script launches adafs on multiple nodes',
                                     formatter_class=argparse.RawTextHelpFormatter)
    # positional arguments
    parser.add_argument('daemonpath', type=str,
                        help='path to the daemon executable')
    parser.add_argument('rootdir', type=str,
                        help='path to the root directory where all data will be stored')
    parser.add_argument('mountdir', type=str,
                        help='path to the mount directory of the file system')
    parser.add_argument('nodelist', type=str,
                        help='''list of nodes where the file system is launched. This can be a comma-separated list
                             or a path to a nodefile (one node per line)''')

    # optional arguments
    parser.add_argument('-p', '--pretend', action='store_true',
                        help='Output adafs launch command and do not actually execute it')
    args = parser.parse_args()

    if args.pretend is True:
        PRETEND = True
    init_system(args.daemonpath, args.rootdir, args.mountdir, args.nodelist)

    print '\nNothing left to do; exiting. :)'
