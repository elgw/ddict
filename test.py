import time

dictfile = "ord.txt"
txtfile = "pg75742.txt"

dict1 = dict()

t0 = time.perf_counter()
with open(dictfile, 'r') as fid:
    for line in fid:
        for word in line.split():
            dict1[word] = 0
t1 = time.perf_counter()
n_known = 0
n_unknown = 0

with open(txtfile, 'r') as fid:
    for line in fid:
        for word in line.split():
            if word in dict1.keys():
                n_known = n_known +1
            else:
                n_unknown = n_unknown + 1
t2 = time.perf_counter()
print(f'Found {n_known} known and {n_unknown} unknown words')
print(f'Construct: {t1-t0:.3f}')
print(f'Scan: {t2-t1:.3f}')
print(f'Total: {t2-t0:.3f}')
