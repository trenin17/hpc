#!/usr/bin/env python3
"""
extract_single_metric.py
Search one PCM text file for the first‑column string
given in TARGET_METRIC and print a CSV line:
<MetricNameWithoutSpaces>,v1,v2,v3,…
(one value for every Metric table found in the file)
"""

import sys
import re
from pathlib import Path

# ─── 1. EDIT THIS CONSTANT ──────────────────────────────────────────────────────
TARGET_METRIC = "DP [MFLOP/s]"          # ← the exact text that appears in the file
# ────────────────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) != 2:
        sys.exit(f"Usage: {sys.argv[0]} pcm_output.txt")

    text = Path(sys.argv[1]).read_text(errors="ignore")

    # Split at the beginnings of “Group 1 Metric” tables
    metric_tables = re.split(r'^TABLE,Region .*?,Group 1 Metric,.*?$',
                             text, flags=re.MULTILINE)

    values = []
    for tbl in metric_tables[1:]:               # first chunk is pre‑table fluff
        for line in tbl.strip().splitlines():
            cols = [c.strip() for c in line.split(',')]
            # Data lines look like:  name , value , (blank)
            if len(cols) >= 2 and cols[0] == TARGET_METRIC:
                values.append(cols[1])          # collect the number
                break                           # only one per table

    # Output
    csv_line = ','.join(values)
    print(csv_line)

if __name__ == "__main__":
    main()
