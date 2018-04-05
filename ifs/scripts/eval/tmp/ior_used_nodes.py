#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

import os

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

nodes = list()


def parse_file(filepath):
    n = 0
    nodes_tmp = list()
    with open(filepath, 'r') as rf:
        for line in rf.readlines():
            if 'Startup successful. Daemon is ready.' in line:
                n += 1
            if '[SUCCESS]' in line:
                nodes_tmp.append(int(line.strip().split()[-1][1:]))
            if 'DAEMON STOP' in line:
                break
    # put create stat and remove into dict index 0: avg, index 1 std
    nodes_tmp.sort(key=lambda x: x)
    nodes.append(nodes_tmp)


def parse_ior_out(inpath, faulty_node_set):
    if not os.path.exists(inpath) or not os.path.isdir(inpath):
        print "Input path does not exist or is not a directory. Exiting."
        exit(1)
    # parse input
    in_depth = inpath.count(os.path.sep)
    for root, dirs, files in os.walk(inpath):
        curr_depth = root.count(os.path.sep)
        if curr_depth > in_depth:
            break
        for file in files:
            filepath = '%s/%s' % (root, file)
            parse_file(filepath)
    nodes.sort(key=lambda x: len(x))
    faulty_index = -1
    for i, n in enumerate(nodes):
        if len(n) == faulty_node_set:
            faulty_index = i
    tmp_set = set(nodes[faulty_index])
    for i, n in enumerate(nodes):
        if i == faulty_index:
            continue
        tmp_set = tmp_set.difference(set(n))
    tmp_set = list(tmp_set)
    tmp_set.sort()
    print ','.join(map(str, tmp_set))


if __name__ == "__main__":
    # Init parser
    parser = argparse.ArgumentParser(description='This scripts returns a sorted number of nodes used in IOR ',
                                     formatter_class=argparse.RawTextHelpFormatter)
    # positional arguments
    parser.add_argument('ior_in_path', type=str,
                        help='path to the ior out input file. If its a directory it will process all files in it.')
    parser.add_argument('nodes_number_to_intersect', type=int,
                        help='nodes_number_to_intersect')

    args = parser.parse_args()

    parse_ior_out(args.ior_in_path, args.nodes_number_to_intersect)

    print '\nNothing left to do; exiting. :)'
