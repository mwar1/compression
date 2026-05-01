import os
import time
import subprocess
import csv
from tqdm import tqdm
from pathlib import Path
import pandas as pd

CORPUS_DIR = "./inputs"
ALGORITHMS = ["RLE", "Huffman", "Range"]
RESULTS_FILE = "benchmark_results.csv"
OUTPUT_FILE = "out.comp"

def run_benchmark():
    files = [str(p) for p in Path(CORPUS_DIR).rglob("*") if p.is_file()]
    
    with open(RESULTS_FILE, mode="w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Filename", "Algorithm", "Original(B)", "Compressed(B)", "Ratio", "Time(ms)"])

        for filename in tqdm(files, desc="Benchmarking"):
            original_size = os.path.getsize(filename)

            for algo in ALGORITHMS:
                start = time.perf_counter()
                
                # Reuses the same output file between runs
                subprocess.run(["./build/compressor", algo, filename, OUTPUT_FILE], capture_output=True)
                
                end = time.perf_counter()
                
                comp_size = os.path.getsize(OUTPUT_FILE)
                duration_ms = (end - start) * 1000
                ratio = comp_size / original_size

                writer.writerow([filename, algo, original_size, comp_size, f"{ratio:.4f}", f"{duration_ms:.2f}"])

    if os.path.exists(OUTPUT_FILE):
        os.remove(OUTPUT_FILE)

def analyse_results():
    df = pd.read_csv(RESULTS_FILE)

    summary = df.groupby("Algorithm").agg({
        "Ratio": "median", # Median to ignore outliers
        "Time(ms)": "mean",
        "Original(B)": "sum",
        "Compressed(B)": "sum"
    }).reset_index()

    # Calculate overall space savings percentage
    summary["Average %"] = (1 - (summary["Compressed(B)"] / summary["Original(B)"])) * 100

    print("# Overall Stats")
    print(summary[["Algorithm", "Ratio", "Average %", "Time(ms)"]].to_string(index=False))
    print("\n")

    # Find the best algo for each file
    pivot_df = df.pivot(index="Filename", columns="Algorithm", values="Ratio")
    
    # Identify which algorithm had the lower (better) ratio
    pivot_df["Winner"] = pivot_df.idxmin(axis=1)
    
    print("# Winner by file")
    print(pivot_df.to_string())
    print("\n")

if __name__ == "__main__":
    run_benchmark()
    analyse_results()