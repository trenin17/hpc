#!/usr/bin/env python3
import sys
import csv

def average_columns(csv_file):
    with open(csv_file, newline='') as f:
        reader = csv.reader(f)
        header = next(reader)
        sums = [0.0] * len(header)
        counts = [0] * len(header)
        for row in reader:
            for i, value in enumerate(row):
                try:
                    val = float(value)
                    sums[i] += val
                    counts[i] += 1
                except ValueError:
                    continue  # Skip non-numeric values

    for col, s, c in zip(header, sums, counts):
        avg = s / c if c > 0 else float('nan')
        print(f"{col}: {avg}")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python average_columns.py file.csv")
    else:
        average_columns(sys.argv[1])
