#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

import numpy as np
import os

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

node_n = list()
mdtest_d = dict(create_avg=list(), stat_avg=list(), remove_avg=list(), create_std=list(), stat_std=list(),
                remove_std=list())


def parse_file(filepath):
    flag = False
    # extraxt relevant info from file and put it into mdtest_out list
    mdtest_out = []
    create_tmp = []
    stat_tmp = []
    remove_tmp = []
    n = 0
    with open(filepath, 'r') as rf:
        for line in rf.readlines():
            if 'Startup successful. Daemon is ready.' in line:
                n += 1
            if 'SUMMARY: (of' in line:
                flag = True
            if '-- finished at ' in line or 'V-1: Entering timestamp...' in line:
                flag = False
                # Filter for relevant stuff
                mdtest_out = mdtest_out[3:-2]
                # put values for iteration in tmp lists
                create_tmp.append(float([x for x in mdtest_out[0].strip().split(' ') if x][-2:-1][0]))
                stat_tmp.append(float([x for x in mdtest_out[1].strip().split(' ') if x][-2:-1][0]))
                remove_tmp.append(float([x for x in mdtest_out[3].strip().split(' ') if x][-2:-1][0]))
            if flag:
                mdtest_out.append(line.strip())
            if 'DAEMON STOP' in line:
                break
    if len(create_tmp) == 0:
        # something is wrong. discard this file
        print 'File %s does not contain mdtest results' % filepath
        return

    node_n.append(n)
    # calc mean and standard deviation
    mdtest_d['create_avg'].append(np.mean(create_tmp))
    mdtest_d['create_std'].append(np.std(create_tmp))
    mdtest_d['stat_avg'].append(np.mean(stat_tmp))
    mdtest_d['stat_std'].append(np.std(stat_tmp))
    mdtest_d['remove_avg'].append(np.mean(remove_tmp))
    mdtest_d['remove_std'].append(np.std(remove_tmp))


def parse_mdtest_out(inpath, outpath='', printshell=False, printonly=True):
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

    # create output
    # first put node number and their mdtest numbers in list and sort them
    csv_l = list()
    for i in range(len(node_n)):
        csv_line = ''
        csv_line += '%s,' % mdtest_d['create_avg'][i]
        csv_line += '%s,' % mdtest_d['stat_avg'][i]
        csv_line += '%s' % mdtest_d['remove_avg'][i]
        csv_line += '%s,' % mdtest_d['create_std'][i]
        csv_line += '%s,' % mdtest_d['stat_std'][i]
        csv_line += '%s\n' % mdtest_d['remove_std'][i]
        csv_l.append([node_n[i], csv_line])
    csv_l.sort(key=lambda x: x[0])
    # convert sorted list into csv text file
    csv = 'n,creates_avg/sec,stats_avg/sec,remove_avg/sec,create_std,stat_std,remove_std\n'
    for i in csv_l:
        csv += '%d,%s' % (i[0], i[1])
    # print output
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
