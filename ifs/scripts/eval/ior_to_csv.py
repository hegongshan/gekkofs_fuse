#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse

import numpy as np
import os

__author__ = "Marc-Andre Vef"
__email__ = "vef@uni-mainz.de"

node_n = list()
results_n = list()


def mebi_to_mega(size):
    return (float(size) * 1024 * 1024) / 1000 / 1000


def transfer_to_mb(transfer_size_unit):
    # size in gigabytes
    if transfer_size_unit[-1] == 'g':
        transfersize_kb = float(transfer_size_unit[:-1]) * 1024
    # size in megabytes
    elif transfer_size_unit[-1] == 'm':
        return float(transfer_size_unit[:-1])
    # size in kilobytes
    elif transfer_size_unit[-1] == 'k':
        return float(transfer_size_unit[:-1]) / 1024
    # size in bytes
    else:
        return float(transfer_size_unit[:-1]) / 1024 / 1024


def parse_file(filepath):
    write_tmp = []
    read_tmp = []
    write_avg = []
    write_std = []
    read_avg = []
    read_std = []
    transfersizes = []
    write_iops_avg = []
    read_iops_avg = []
    n = 0
    curr_transfer_size_unit = ''
    with open(filepath, 'r') as rf:
        for line in rf.readlines():
            if 'Startup successful. Daemon is ready.' in line:
                n += 1
            if '<new_transfer_size>' in line:
                curr_transfer_size_unit = line.strip().split(';')[1]
                write_tmp = []
                read_tmp = []
            if '<finish_transfer_size>' in line:
                transfersizes.append(curr_transfer_size_unit)
                # calc average throughput
                write_avg.append(np.mean(write_tmp))
                write_std.append(np.std(write_tmp))
                # calc average standard deviation over all iterations
                read_avg.append(np.mean(read_tmp))
                read_std.append(np.std(read_tmp))
                # calc write and read operations per second (i.e. IOPS)
                transfer_norm = transfer_to_mb(curr_transfer_size_unit)
                write_avg_mb = mebi_to_mega(write_avg[-1])
                write_iops_avg.append(write_avg_mb / transfer_norm)
                read_avg_mb = mebi_to_mega(read_avg[-1])
                read_iops_avg.append(read_avg_mb / transfer_norm)
                curr_transfer_size_unit = ''
            if 'Max Write' in line:
                write_tmp.append(float(line.split(' ')[2]))
            if 'Max Read' in line:
                read_tmp.append(float(line.split(' ')[3]))
            if 'DAEMON STOP' in line:
                break
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
    tmp_d['write_iops_avg'] = write_iops_avg
    tmp_d['read_iops_avg'] = read_iops_avg
    tmp_d['node_n'] = n
    results_n.append(tmp_d)


def parse_ior_out(inpath, outpath='', printshell=False, printonly=True, calc_iops=True, calc_std=False):
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
    csv_write_iops_avg_l = list()
    csv_read_iops_avg_l = list()
    header_string = '# nodes,%s' % ','.join([x for x in results_n[0]['transfersizes']])
    for i in range(len(node_n)):
        csv_write_avg = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['write_avg']]))
        csv_write_avg_l.append([node_n[i], csv_write_avg])
        csv_read_avg = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['read_avg']]))
        csv_read_avg_l.append([node_n[i], csv_read_avg])
        csv_write_std = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['write_std']]))
        csv_write_std_l.append([node_n[i], csv_write_std])
        csv_read_std = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['read_std']]))
        csv_read_std_l.append([node_n[i], csv_read_std])
        csv_write_iops_avg = '%s,%s' % (
        node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['write_iops_avg']]))
        csv_write_iops_avg_l.append([node_n[i], csv_write_iops_avg])
        csv_read_iops_avg = '%s,%s' % (node_n[i], ','.join(["{:.2f}".format(x) for x in results_n[i]['read_iops_avg']]))
        csv_read_iops_avg_l.append([node_n[i], csv_read_iops_avg])
    # sort by number of nodes and create csvs
    csv_write_avg_l.sort(key=lambda x: x[0])
    csv_write_avg = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_write_avg_l]))
    csv_read_avg_l.sort(key=lambda x: x[0])
    csv_read_avg = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_read_avg_l]))
    csv_write_std_l.sort(key=lambda x: x[0])
    csv_write_std = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_write_std_l]))
    csv_read_std_l.sort(key=lambda x: x[0])
    csv_read_std = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_read_std_l]))
    csv_write_iops_avg_l.sort(key=lambda x: x[0])
    csv_write_iops_avg = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_write_iops_avg_l]))
    csv_read_iops_avg_l.sort(key=lambda x: x[0])
    csv_read_iops_avg = '%s\n%s' % (header_string, '\n'.join([x[1] for x in csv_read_iops_avg_l]))
    # print output
    if printshell:
        print 'Write_avg:'
        print csv_write_avg
        print '\nRead_avg:'
        print csv_read_avg
        if calc_std:
            print '\nWrite_std:'
            print csv_write_std
            print '\nRead_std:'
            print csv_read_std
        if calc_iops:
            print '\nWrite_IOPS_avg:'
            print csv_write_iops_avg
            print '\nRead_IOPS_avg:'
            print csv_read_iops_avg

    if not printonly and outpath != '':
        # write output
        with open(outpath, 'w') as wf:
            wf.write('Write_avg:')
            wf.write(csv_write_avg)
            wf.write('Read_avg:')
            wf.write(csv_read_avg)
            if calc_std:
                wf.write('Write_std:')
                wf.write(csv_write_std)
                wf.write('Read_std:')
                wf.write(csv_read_std)
            if calc_iops:
                wf.write('Write_IOPS_avg:')
                wf.write(csv_write_iops_avg)
                wf.write('Read_IOPS_avg:')
                wf.write(csv_read_iops_avg)


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
    parser.add_argument('-s', '--std', action='store_true',
                        help='Set to output standard deviation')
    parser.add_argument('-i', '--iops', action='store_true',
                        help='Enable calculating write and read oprations per second')

    args = parser.parse_args()
    calc_std = True if args.std else False
    calc_iops = True if args.iops else False
    print_shell = False
    print_only = False
    out_path = ''
    if args.printshell and args.output != '' and not args.printonly:
        print_shell = True
        print_only = False
        out_path = args.output
    elif args.output != '' and not args.printonly:
        print_shell = False
        print_only = False
        out_path = args.output
    else:
        print_shell = True
        print_only = True

    parse_ior_out(args.ior_in_path, out_path, print_shell, print_only, calc_iops, calc_std)

    print '\nNothing left to do; exiting. :)'
