import glob
import re
import matplotlib.pyplot as plt

# Find all result files
files = glob.glob("results/omp_close_*.out")
files_ft = glob.glob("results/omp_spread_*.out")

x_vals = []
y_vals = []

x_vals_ft = []
y_vals_ft = []

# Regular results
for fname in files:
    match = re.search(r'omp_close_(\d+)\.out', fname)
    if not match:
        continue
    threads = int(match.group(1))
    with open(fname) as f:
        lines = f.readlines()
        if len(lines) < 12:
            continue
        line = lines[11].strip()
        parts = line.split(';')
        if len(parts) < 2:
            continue
        try:
            time_val = float(parts[1])
        except ValueError:
            continue
        speedup = 2.385 / time_val
        x_vals.append(threads)
        y_vals.append(speedup)

# FT results
for fname in files_ft:
    match = re.search(r'omp_spread_(\d+)\.out', fname)
    if not match:
        continue
    threads = int(match.group(1))
    with open(fname) as f:
        lines = f.readlines()
        if len(lines) < 12:
            continue
        line = lines[11].strip()
        parts = line.split(';')
        if len(parts) < 2:
            continue
        try:
            time_val = float(parts[1])
        except ValueError:
            continue
        speedup = 2.363 / time_val
        x_vals_ft.append(threads)
        y_vals_ft.append(speedup)

# Sort by thread count
xy = sorted(zip(x_vals, y_vals))
x_sorted, y_sorted = zip(*xy)

xy_ft = sorted(zip(x_vals_ft, y_vals_ft))
if xy_ft:
    x_sorted_ft, y_sorted_ft = zip(*xy_ft)
else:
    x_sorted_ft, y_sorted_ft = [], []

plt.figure()
plt.plot(x_sorted, y_sorted, marker='o', label='Close')
if x_sorted_ft:
    plt.plot(x_sorted_ft, y_sorted_ft, marker='s', label='Spread')
plt.xlabel('Thread count')
plt.ylabel('Speedup (relative to 1 thread)')
plt.title('omp Speedup 6000')
plt.grid(True)
plt.legend()
plt.savefig("sc6000.png", dpi=300, bbox_inches='tight')
plt.show()