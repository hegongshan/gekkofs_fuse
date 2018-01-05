#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import time

import os

from util import util

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

global PRETEND
global PSSH_PATH
global WAITTIME

CONST_PSSH_HOSTFILE_PATH = '/tmp/hostfile_pssh'


def check_dependencies():
    global PSSH_PATH
    """Check if pssh is installed"""
    pssh_path = os.popen('which pssh').read().strip()
    if pssh_path != '':
        PSSH_PATH = pssh_path
        return
    pssh_path = os.popen('which parallel-ssh').read().strip()
    if pssh_path != '':
        PSSH_PATH = pssh_path
        return
    print '[ERR] parallel-ssh/pssh executable cannot be found. Please add it to the parameter list'
    exit(1)


def init_system(daemon_path, rootdir, mountdir, nodelist, cleanroot):
    """Initializes ADAFS on specified nodes.

    Args:
        daemon_path (str): Path to daemon executable
        rootdir (str): Path to root directory for fs data
        mountdir (str): Path to mount directory where adafs is used in
        nodelist (str): Comma-separated list of nodes where adafs is launched on
    """
    global PSSH_PATH
    global PRETEND
    # get absolute paths
    daemon_path = os.path.realpath(os.path.expanduser(daemon_path))
    mountdir = os.path.realpath(os.path.expanduser(mountdir))
    rootdir = os.path.realpath(os.path.expanduser(rootdir))
    pssh_nodelist = ''
    if not os.path.exists(daemon_path) or not os.path.isfile(daemon_path):
        print '[ERR] Daemon executable not found or not a file'
        exit(1)
    nodefile = False
    if os.path.exists(nodelist):
        nodefile = True
        if not util.create_pssh_hostfile(nodelist, CONST_PSSH_HOSTFILE_PATH):
            exit(1)
        nodelist = CONST_PSSH_HOSTFILE_PATH
    if PSSH_PATH is '':
        check_dependencies()
    # set pssh arguments
    if nodefile:
        pssh = '%s -O StrictHostKeyChecking=no -i -h "%s"' % (PSSH_PATH, nodelist)
    else:
        pssh = '%s -O StrictHostKeyChecking=no -i -H "%s"' % (PSSH_PATH, nodelist.replace(',', ' '))

    # clean root dir if needed
    if cleanroot:
        cmd_rm_str = '%s "rm -rf %s/*"' % (pssh, rootdir)
        if PRETEND:
            print 'Pretending: %s' % cmd_rm_str
        else:
            print 'Running: %s' % cmd_rm_str
            pssh_ret = util.exec_shell(cmd_rm_str, True)
            err = False
            for line in pssh_ret:
                if 'FAILURE' in line.strip()[:30]:
                    err = True
                    print '------------------------- ERROR pssh -- Host "%s" -------------------------' % \
                          (line[line.find('FAILURE'):].strip().split(' ')[1])
                    print line
            if not err:
                print 'pssh daemon launch successfully executed. Root dir is cleaned.\n'
            else:
                print '[ERR] with pssh. Aborting!'
                exit(1)

    # Start deamons
    if nodefile:
        cmd_str = '%s "nohup %s -r %s -m %s --hostfile %s > /tmp/adafs_daemon.log 2>&1 &"' \
                  % (pssh, daemon_path, rootdir, mountdir, nodelist)
    else:
        cmd_str = '%s "nohup %s -r %s -m %s --hosts %s > /tmp/adafs_daemon.log 2>&1 &"' \
                  % (pssh, daemon_path, rootdir, mountdir, nodelist)
    if PRETEND:
        print 'Pretending: %s' % cmd_str
    else:
        print 'Running: %s' % cmd_str
        pssh_ret = util.exec_shell(cmd_str, True)
        err = False
        for line in pssh_ret:
            if 'FAILURE' in line.strip()[:30]:
                err = True
                print '------------------------- ERROR pssh -- Host "%s" -------------------------' % \
                      (line[line.find('FAILURE'):].strip().split(' ')[1])
                print line
        if not err:
            print 'pssh daemon launch successfully executed. Checking for FS startup errors ...\n'
        else:
            print '[ERR] with pssh. Aborting. Please run shutdown_adafs.py to shut down orphan adafs daemons!'
            exit(1)

    if not PRETEND:
        print 'Give it some time (%d second) to startup ...' % WAITTIME
        for i in range(WAITTIME):
            print '%d\r' % (WAITTIME - i),
            time.sleep(1)

    # Check adafs logs for errors
    cmd_chk_str = '%s "head -6 /tmp/adafs_daemon.log"' % pssh
    if PRETEND:
        print 'Pretending: %s' % cmd_chk_str
    else:
        print 'Running: %s' % cmd_chk_str
        pssh_ret = util.exec_shell(cmd_chk_str, True)
        err = False
        fs_err = False
        for line in pssh_ret:
            if 'Failure' in line.strip()[:30]:
                err = True
                print '------------------------- ERROR pssh -- Host "%s" -------------------------' % \
                      (line[line.find('FAILURE'):].strip().split(' ')[1])
                print line
            else:
                # check for errors in log
                if '[E]' in line[line.strip().find('\n') + 1:] or 'Assertion `err\'' in line[
                                                                                          line.strip().find('\n') + 1:]:
                    fs_err = True
                    print '------------------------- ERROR pssh -- Host "%s" -------------------------' % \
                          (line.strip().split(' ')[3].split('\n')[0])
                    print '%s' % line[line.find('\n') + 1:]

        if not err and not fs_err:
            print 'pssh logging check successfully executed. Looks prime.'
        else:
            print '[ERR] while checking fs logs. Aborting. Please run shutdown_adafs.py to shut down orphan adafs daemons!'
            exit(1)


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
    parser.add_argument('-P', '--pssh', metavar='<PSSH_PATH>', type=str, default='',
                        help='Path to parallel-ssh/pssh. Defaults to /usr/bin/{parallel-ssh,pssh}')
    parser.add_argument('-c', '--cleanroot', action='store_true',
                        help='Removes contents of root directory before starting ADA-FS Daemon. Be careful!')
    args = parser.parse_args()
    if args.pretend:
        PRETEND = True
    else:
        PRETEND = False
    PSSH_PATH = args.pssh
    WAITTIME = 5
    init_system(args.daemonpath, args.rootdir, args.mountdir, args.nodelist, args.cleanroot)

    print '\nNothing left to do; exiting. :)'
