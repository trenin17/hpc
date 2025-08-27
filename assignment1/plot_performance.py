import pandas as pd
import matplotlib.pyplot as plt
import sys

def plot_csv(filename, output_file='plot_performance.png'):
    # Read CSV
    df = pd.read_csv(filename)

    # Extract resolution columns (skip first and last columns)
    res_cols = [col for col in df.columns if col.startswith('Resolution_')]
    x_vals = [int(col.split('_')[1]) for col in res_cols]

    # Create plot
    fig, ax = plt.subplots(figsize=(10, 6))

    # Plot each optimization flag
    for _, row in df.iterrows():
        y_vals = [row[col] for col in res_cols]
        ax.plot(x_vals, y_vals, 'o-', label=row.iloc[0], linewidth=2, markersize=6)

    # Style the plot
    ax.set_xlabel('Resolution', fontsize=12)
    ax.set_ylabel('MFlops', fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')

    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Plot saved to {output_file}")

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else 'performance_data.csv'
    plot_csv(filename)
