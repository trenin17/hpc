#!/usr/bin/env python3
import json
import matplotlib.pyplot as plt

def plot_from_json(json_file, output_png="plot.png"):
    with open(json_file, 'r') as f:
        data = json.load(f)

    # Extract configurations (x-axis, using string keys)
    configs = sorted(list(next(iter(data.values())).keys()), key=lambda x: float(x))
    x = [float(c) for c in configs]

    # Get data series
    mflops = [data['DP MFLOP/s'][c] for c in configs]
    l2_miss = [data['L2 miss rate'][c] for c in configs]
    l3_miss = [data['L3 miss rates'][c] for c in configs]

    fig, ax1 = plt.subplots(figsize=(8, 5))

    # Plot DP MFLOP/s on left y-axis
    color1 = 'tab:red'
    p1, = ax1.plot(x, mflops, color=color1, linewidth=2, label='DP MFLOP/s')
    ax1.set_xlabel('Configurations')
    ax1.set_ylabel('Mflops', color=color1)
    ax1.tick_params(axis='y', labelcolor=color1)

    # Create a second y-axis for L2/L3 miss rates
    ax2 = ax1.twinx()
    color2 = 'tab:blue'
    color3 = 'tab:orange'
    p2, = ax2.plot(x, [v*100 for v in l2_miss], color=color2, linewidth=2, label='L2 miss rate (%)')
    p3, = ax2.plot(x, [v*100 for v in l3_miss], color=color3, linewidth=2, label='L3 miss rate (%)')
    ax2.set_ylabel('L2/L3 miss rate in %', color=color2)
    ax2.tick_params(axis='y', labelcolor=color2)

    # Add legend
    lines = [p1, p2, p3]
    labels = [l.get_label() for l in lines]
    plt.legend(lines, labels, loc='upper left', frameon=False)

    # Add annotation for test info
    plt.gcf().text(
        0.5, -0.08, "Average over 10 runs on test node with -O3 -fno-alias -xhost flags.",
        fontsize=10, ha='center', va='center'
    )

    # Custom styling to mimic sketch
    ax1.spines['top'].set_visible(False)
    ax1.spines['right'].set_visible(False)
    ax2.spines['top'].set_visible(False)

    plt.tight_layout()
    plt.savefig(output_png, format='png', bbox_inches='tight', dpi=300)
    print(f"Plot saved as {output_png}")

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print("Usage: python plot_performance_json.py data.json [output.png]")
    else:
        output_file = sys.argv[2] if len(sys.argv) > 2 else "plot.png"
        plot_from_json(sys.argv[1], output_file)
