###
#  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
#  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

#  This software was partially supported by the
#  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

#  This software was partially supported by the
#  ADA-FS project under the SPPEXA project funded by the DFG.

#  SPDX-License-Identifier: MIT
###
#!/usr/bin/python3
import re
import sys
import collections

file = sys.argv[1]

pattern = re.compile(r".+(read\ )(.*)( host: )(\d+).+(path: )(.+),.+(chunk_start: )(\d+).+(chunk_end: )(\d+)")

d = collections.OrderedDict()

with open(file) as f:
    for line in f:
        result = pattern.match(line)
        if result:
            d[result.group(2)] = 1
keys = sorted(d.keys())
i = 0
for key in keys:
    d[key] = i
    i = i + 1

with open(file) as f:
    for line in f:
        result = pattern.match(line)
        if result:
            for i in range(int(result.group(8)), int(result.group(10))+1):
                print (result.group(6), i, d[result.group(2)])
