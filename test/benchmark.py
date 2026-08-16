import subprocess
import time

binaries = ['./ddict_test', './glib_test']

def run_bin(bin, N):
    cmd = [bin, str(N)]
    res = subprocess.run(cmd, capture_output=True)
    stdout = res.stdout.decode().split('\n')
    result = {}
    result['binary'] = bin
    result['N'] = N


    for line in stdout:
        parts = line.split(' ')
        if parts[0] == 'Scan:':
            result['t_scan'] = float(parts[1])
        if parts[0] == 'Insert:':
            result['t_insert'] = float(parts[1])
        if parts[0] == 'Total:':
            result['t_total'] = float(parts[1])
        if parts[0] == 'VmHWM:':
            result['VmHWM'] = float(parts[1])
    return result


def benchmark(binary, N, timeout):
    res = []
    t0 = time.time()
    dt = 0
    while dt < timeout:
        res.append(run_bin(binary, N))
        dt = time.time() - t0
    properties = ['t_insert', 't_scan', 't_total', 'VmHWM']
    summary = {}
    for prop in properties:
        values = [x[prop] for x in res]
        summary[prop] = min(values)
    
    print(f'| {binary} | {N:.0e} | {summary['t_insert']:,.0f} | {summary['t_scan']:,.0f} | {summary['t_total']:,.0f} | {summary['VmHWM']/1000:,.0f} |')

if __name__ == '__main__':

    cols = ['method', 'N', 't_create [ms]', 't_scan [ms]', 't_total [ms]', 'VmHWM [MB]']
    print('| ' + ' | '.join(cols) + ' |')
    for i, _ in enumerate(cols):
        if i == 0:
            print('| --- |', end='')
        else:
            print(' ---:|', end='')
    print('')
    
    N0 = 1e5
    N1 = 1e7
    timeout = 2
    
    for binary in binaries:
        N = N0
        while N <= N1:
            benchmark(binary, N, timeout)
            N = N*10
            
