#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

import os

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

node_n = list()
mdtest_d = dict(create=list(), stat=list(), remove=list())


def parse_file(filepath):
    flag = False
    # extraxt relevant info from file and put it into mdtest_out list
    mdtest_out = []
    n = -1
    with open(filepath, 'r') as rf:
        for line in rf.readlines():
            if 'mdtest-1.9.3 was launched with' in line:
                n = line.strip().split(' ')[-2]
            if 'SUMMARY: (of' in line:
                flag = True
            if '-- finished at ' in line or 'V-1: Entering timestamp...' in line:
                flag = False
            if flag:
                mdtest_out.append(line.strip())

    # Filter for relevant stuff
    mdtest_out = mdtest_out[3:-2]
    # put create stat and remove into dict index 0: avg, index 1 std
    node_n.append(n)
    mdtest_d['create'].append(','.join([x for x in mdtest_out[0].strip().split(' ') if x][-2:]))
    mdtest_d['stat'].append(','.join([x for x in mdtest_out[1].strip().split(' ') if x][-2:]))
    mdtest_d['remove'].append(','.join([x for x in mdtest_out[3].strip().split(' ') if x][-2:]))


def parse_mdtest_out(inpath, outpath='', printshell=False, printonly=True):
    if not os.path.exists(inpath) or not os.path.isdir(inpath):
        print "Input path does not exist or is not a directory. Exiting."
        exit(1)
    # parse input
    for root, dirs, files in os.walk(inpath):
        for file in files:
            filepath = '%s/%s' % (root, file)
            parse_file(filepath)
    # create output
    csv = 'n,creates/sec,create_std,stats/sec,stat_std,remove/sec,remove_std\n'
    for i in range(len(node_n)):
        csv += '%s,' % node_n[i]
        csv += '%s,' % mdtest_d['create'][i]
        csv += '%s,' % mdtest_d['stat'][i]
        csv += '%s\n' % mdtest_d['remove'][i]

    if printshell:
        print csv
    if not printonly and outpath != '':
        # write output
        with open(outpath, 'w') as wf:
            wf.write(csv)


if __name__ == "__main__":
    # Init parser
    parser = argparse.ArgumentParser(description='This script converts an mdtest output file into a csv. '
                                                 'If only input path is given, csv is printed on shell',
                                     formatter_class=argparse.RawTextHelpFormatter)
    # positional arguments
    parser.add_argument('mdtest_in_path', type=str,
                        help='path to the mdtest out input file. If its a directory it will process all files in it.')
    parser.add_argument('-o', '--output', metavar='<outpath>', type=str, default='',
                        help='path to the csv output file location')
    parser.add_argument('-p', '--printshell', action='store_true',
                        help='Output csv on shell')
    parser.add_argument('--printonly', action='store_true',
                        help='Only output csv on shell')
    args = parser.parse_args()
    if args.printshell and args.output != '' and not args.printonly:
        parse_mdtest_out(args.mdtest_in_path, args.output, True, False)
    elif args.output != '' and not args.printonly:
        parse_mdtest_out(args.mdtest_in_path, args.output, False, False)
    else:
        parse_mdtest_out(args.mdtest_in_path, '', True, True)

    print '\nNothing left to do; exiting. :)'
