import subprocess
import time
import sys
import argparse


binaries = ['./ddict_test',
            './glib_test',
            './cdict_test',
            './go_test',
            './test/pydict.py',
            './libcdict_test']
bin_alias = {}
bin_alias['./ddict_test'] = 'ddict'
bin_alias['./glib_test'] = 'glib'
bin_alias['./cdict_test'] = 'cdict.h'
bin_alias['./go_test'] = 'go'
bin_alias['./test/pydict.py'] = 'Python'
bin_alias['./libcdict_test'] = 'libcdict'

properties = ['t_insert', 't_scan', 't_total', 'VmHWM']
prop_alias = ['Insert:', 'Scan:', 'Total:', 'VmHWM:']

def run_bin(bin, N):
    assert(N > 100)
    cmd = [bin, str(int(N))]

    try:
        res = subprocess.run(cmd, capture_output=True)
    except FileNotFoundError:
        printf("Could not run the binary named [cmd] -- did you forget to compile?")
        sys.exit(1)

    stdout = res.stdout.decode().split('\n')
    result = {}
    result['binary'] = bin
    result['N'] = N

    for line in stdout:
        line = ' '.join(line.split())
        parts = line.split(' ')
        for prop, alias in zip(properties, prop_alias):
            if parts[0] == alias:
                result[prop] = float(parts[1])

    for prop, alias in zip(properties, prop_alias):
        if not prop in result.keys():
            print(f'Could not get {prop} from binary {bin}')
            print(f'No line starting with {alias} in the output')
            print(f'Command {' '.join(cmd)}')
            print(result)
            sys.exit(1)

    return result


def benchmark(binary, N, timeout):
    res = []
    t0 = time.time()
    dt = 0
    while dt < timeout:
        res.append(run_bin(binary, N))
        dt = time.time() - t0

    summary = {}
    for prop in properties:
        values = [x[prop] for x in res]
        summary[prop] = min(values)

    balias = bin_alias[binary]
    print(f'| {balias} | {N:.0e} | {summary['t_insert']:,.0f} | {summary['t_scan']:,.0f} | {summary['t_total']:,.0f} | {summary['VmHWM']/1000:,.0f} |')

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument("--Nmax", default=1e8, type=int, help="Largest number of elements to insert", required=False)
    args = parser.parse_args()

    cols = ['method', 'N', 't_create [ms]', 't_scan [ms]', 't_total [ms]', 'VmHWM [MB]']
    print('| ' + ' | '.join(cols) + ' |')
    for i, _ in enumerate(cols):
        if i == 0:
            print('| --- |', end='')
        else:
            print(' ---:|', end='')
    print('')

    N0 = 1e5
    N1 = args.Nmax
    timeout = 2

    for binary in binaries:
        N = N0
        while N <= N1:
            benchmark(binary, N, timeout)
            N = N*10
