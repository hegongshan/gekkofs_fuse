#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os

import numpy as np

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

node_n = list()
results_n = list()


def parse_file(filepath):
    write_tmp = []
    read_tmp = []
    write_avg = []
    write_std = []
    read_avg = []
    read_std = []
    transfersizes = []
    n = 0
    curr_transfer = ''
    with open(filepath, 'r') as rf:
        for line in rf.readlines():
            if 'Startup successful. Daemon is ready.' in line:
                n += 1
            if '<new_transfer_size>' in line:
                curr_transfer = line.strip().split(';')[1]
                write_tmp = []
                read_tmp = []
            if '<finish_transfer_size>' in line:
                transfersizes.append(curr_transfer)
                write_avg.append(np.mean(write_tmp))
                write_std.append(np.std(write_tmp))
                read_avg.append(np.mean(read_tmp))
                read_std.append(np.std(read_tmp))
                curr_transfer = ''
            if 'Max Write' in line:
                write_tmp.append(float(line.split(' ')[2]))
            if 'Max Read' in line:
                read_tmp.append(float(line.split(' ')[3]))
    if len(write_avg) == 0 or len(read_avg) == 0:
        # something is wrong. discard this file
        print 'File %s does not contain results' % filepath
        return
    # put create stat and remove into dict index 0: avg, index 1 std
    node_n.append(n)
    tmp_d = dict()
    tmp_d['transfersizes'] = transfersizes
    tmp_d['write_avg'] = write_avg
    tmp_d['read_avg'] = read_avg
    tmp_d['write_std'] = write_std
    tmp_d['read_std'] = read_std
    tmp_d['node_n'] = n
    results_n.append(tmp_d)


def parse_ior_out(inpath, outpath='', printshell=False, printonly=True):
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

    # create csv output
    csv_write_avg_l = list()
    csv_read_avg_l = list()
    csv_write_std_l = list()
    csv_read_std_l = list()
    header_string = '# nodes,%s' % ','.join([x for x in results_n[0]['transfersizes']])
    for i in range(len(node_n)):
        csv_write_avg = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['write_avg']]))
        csv_write_std = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['write_std']]))
        csv_read_avg = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['read_avg']]))
        csv_read_std = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['read_std']]))
        csv_write_avg_l.append([node_n[i], csv_write_avg])
        csv_read_avg_l.append([node_n[i], csv_read_avg])
        csv_write_std_l.append([node_n[i], csv_write_std])
        csv_read_std_l.append([node_n[i], csv_read_std])
    # sort by number of nodes
    csv_write_avg_l.sort(key=lambda x: x[0])
    csv_read_avg_l.sort(key=lambda x: x[0])
    csv_write_std_l.sort(key=lambda x: x[0])
    csv_read_std_l.sort(key=lambda x: x[0])
    # create csv strings
    csv_write_avg = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_write_avg_l]))
    csv_write_std = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_write_std_l]))
    csv_read_avg = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_read_avg_l]))
    csv_read_std = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_read_std_l]))
    # print output
    if printshell:
        print 'Write_avg:'
        print csv_write_avg
        print '\nRead_avg:'
        print csv_read_avg
        print '\nWrite_std:'
        print csv_write_std
        print '\nRead_std:'
        print csv_read_std
    if not printonly and outpath != '':
        # write output
        with open(outpath, 'w') as wf:
            wf.write('Write_avg:')
            wf.write(csv_write_avg)
            wf.write('Read_avg:')
            wf.write(csv_read_avg)
            wf.write('Write_std:')
            wf.write(csv_write_std)
            wf.write('Read_std:')
            wf.write(csv_read_std)


if __name__ == "__main__":
    # Init parser
    parser = argparse.ArgumentParser(description='This script converts an ior output file into a csv. '
                                                 'If only input path is given, csv is printed on shell',
                                     formatter_class=argparse.RawTextHelpFormatter)
    # positional arguments
    parser.add_argument('ior_in_path', type=str,
                        help='path to the ior out input file. If its a directory it will process all files in it.')
    parser.add_argument('-o', '--output', metavar='<outpath>', type=str, default='',
                        help='path to the csv output file location')
    parser.add_argument('-p', '--printshell', action='store_true',
                        help='Output csv on shell')
    parser.add_argument('--printonly', action='store_true',
                        help='Only output csv on shell')
    args = parser.parse_args()
    if args.printshell and args.output != '' and not args.printonly:
        parse_ior_out(args.ior_in_path, args.output, True, False)
    elif args.output != '' and not args.printonly:
        parse_ior_out(args.ior_in_path, args.output, False, False)
    else:
        parse_ior_out(args.ior_in_path, '', True, True)

    print '\nNothing left to do; exiting. :)'
