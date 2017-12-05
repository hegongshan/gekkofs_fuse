#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Script contains different useful functions
# Marc-Andre Vef
# Version 0.2 (06/19/2015)

import collections
import shutil
import sys
import time
from subprocess import Popen, PIPE

import os


def rm_trailing_slash(path):
    """Removes the trailing slash from a given path"""
    return path[:-1] if path[-1] == '/' else path


def create_dir(path):
    """creates a directory at the paths location. Must be an absolute path"""
    try:
        if not os.path.exists(path):
            os.makedirs(path)
    except OSError as e:
        print 'Error: Output directory could not be created.'
        print e.strerror
        sys.exit(1)


def rm_rf(path):
    """rm -rf path"""
    try:
        shutil.rmtree(path)
    except shutil.Error as e:
        print 'Warning: Could not delete path %s' % path
        print e.strerror


def rm_file(path):
    """remove a file"""
    try:
        os.remove(path)
    except OSError as e:
        print 'Warning: Could not delete file %s' % path
        print e.strerror


def tprint(toprint, nobreak=False):
    """Function adds current time to input and prints it to the terminal

    Args:
        toprint (str): The string to be printed.
        nobreak (bool): True if no line break should occur after print. Defaults to False.

    """
    curr_time = time.strftime('[%H:%M:%S]')
    if nobreak:
        print '%s\t%s' % (curr_time, toprint),
    else:
        print '%s\t%s' % (curr_time, toprint)


def exec_shell(cmd, suppress_output=False):
    """Function executes a cmd on the shell

    Args:
        cmd (str): The command to be executed.
        suppress_output (bool): Suppresses command line output. Defaults to False.

    Returns:
        (namedtuple(shell_out[err=(str), output=(str))): Executed cmd output in output
                                                         Err is filled if a cmd error is encountered else ''
    Raises:
        OSError: If command could not be executed.
    """
    try:
        if cmd == '':
            raise OSError("Command string is empty.")
        # simulate change directory to where mdtest executable is located
        p = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, bufsize=1)
        shell_out = ''
        # Poll process for new output until finished
        for line in iter(p.stdout.readline, ''):
            if not suppress_output:
                curr_time = time.strftime('[%H:%M:%S]  ')
                sys.stdout.write('%s%s' % (curr_time, line))
                sys.stdout.flush()
            shell_out += line
        stdout, stderr = p.communicate()
        out_tuple = collections.namedtuple('shell_out', ['err', 'output'])
        return out_tuple(err=stderr.strip(), output=shell_out.strip())
    except OSError as e:
        print 'ERR when executing shell command'
        print e.strerror


def check_shell_out(msg):
    """Function looks for an error in namedtuple of exec_shell and returns the output string only if valid

        Args:
            msg((namedtuple(shell_out[err=(str), output=(str)))): The message to be printed

        Returns:
            (str): Returns the msg output if no error

        Raises:
            OSError: if shell_out contains an error
    """
    if msg.err != '':
        raise OSError('Shell command failed with\n\t%s' % msg.err)
    return msg.output
