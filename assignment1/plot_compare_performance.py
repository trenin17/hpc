import pandas as pd
import matplotlib.pyplot as plt
import sys

def plot_comparison(batch_file, login_file, output_file='plot_compare_performance.png'):
    # Read CSV files
    batch_df = pd.read_csv(batch_file)
    login_df = pd.read_csv(login_file)
    
    # Extract resolution columns
    res_cols = [col for col in batch_df.columns if col.startswith('Resolution_')]
    resolutions = [int(col.split('_')[1]) for col in res_cols]
    
    # Create subplots
    fig, axes = plt.subplots(1, len(res_cols), figsize=(15, 6))
    if len(res_cols) == 1:
        axes = [axes]
    
    # Create box plot for each resolution
    for i, (res_col, resolution) in enumerate(zip(res_cols, resolutions)):
        data = [batch_df[res_col], login_df[res_col]]
        labels = ['Batch Node', 'Login Node']
        
        axes[i].boxplot(data, labels=labels)
        axes[i].set_title(f'Resolution {resolution}')
        axes[i].set_ylabel('MFlop/s')
        axes[i].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Comparison plot saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compare_performance.py batch_results.csv login_results.csv")
        sys.exit(1)
    
    plot_comparison(sys.argv[1], sys.argv[2])  
