#!/bin/env python

import time
import sys

def test_dict(dictfile, txtfile):
    dict1 = dict()

    t0 = time.perf_counter()
    with open(dictfile, 'r') as fid:
        for line in fid:
            dict1[line.strip()] = 0
    t1 = time.perf_counter()
    n_known = 0
    n_unknown = 0
    print(f'Added {len(dict1)} words to the dictionary')

    with open(txtfile, 'r') as fid:
        for line in fid:
            for word in line.split():
                if word in dict1.keys():
                    n_known = n_known +1
                else:
                    n_unknown = n_unknown + 1

    t2 = time.perf_counter()

    print(f'Scanned {n_known+n_unknown} found {n_known} known and {n_unknown}')

    print(f'Construct: {1000*(t1-t0):.3f} ms')
    print(f'Scan: {1000*(t2-t1):.3f} ms')
    print(f'Total: {1000*(t2-t0):.3f} ms')

if __name__ == '__main__':

    dictfile = "dictwords.txt"
    txtfile = "text.txt"
    if len(sys.argv) > 1:
        dictfile = sys.argv[1]
    if len(sys.argv) > 2:
        txtfile = sys.argv[2]
    with open('/proc/self/status') as fid:
        for line in fid:
            if 'VmHWM' in line:
                print(line)
    test_dict(dictfile, txtfile)
    with open('/proc/self/status') as fid:
        for line in fid:
            if 'VmHWM' in line:
                print(line)
