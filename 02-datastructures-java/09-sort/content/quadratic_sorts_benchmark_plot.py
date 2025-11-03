# Reproducible script for building the same table and both plots
# Usage: python quadratic_sorts_benchmark_plot.py

import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

N0 = 100000
baseline = {'Bubble': 13.328, 'Selection': 2.603, 'Insertion': 0.982}

Ns = np.array([1000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000, 10000000], dtype=np.int64)

def to_hms(seconds: float) -> str:
    if not math.isfinite(seconds):
        return "N/A"
    sec = int(round(seconds))
    h = sec // 3600
    m = (sec % 3600) // 60
    s = sec % 60
    return f"{h:02d}:{m:02d}:{s:02d}"

rows = []
for N in Ns:
    row = {"N": int(N)}
    for alg, t0 in baseline.items():
        tN = t0 * (N / N0) ** 2
        row[f"{alg}_s"] = tN
        row[f"{alg}_hms"] = to_hms(tN)
    rows.append(row)

df = pd.DataFrame(rows)

print("\n== Quadratic sorts — seconds ==")
print(df[["N", "Bubble_s", "Selection_s", "Insertion_s"]].to_string(index=False))

print("\n== Quadratic sorts — HH:MM:SS ==")
print(df[["N", "Bubble_hms", "Selection_hms", "Insertion_hms"]].to_string(index=False))

# Plot 1: Linear scale
plt.figure(figsize=(8, 6))
for alg in ["Bubble", "Selection", "Insertion"]:
    plt.plot(df["N"], df[f"{alg}_s"], marker="o", label=alg)
plt.title("Quadratic Sorting (Θ(n²)) — Linear scale")
plt.xlabel("N (input size)")
plt.ylabel("Time (seconds)")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()

# Plot 2: Log–log scale
plt.figure(figsize=(8, 6))
for alg in ["Bubble", "Selection", "Insertion"]:
    plt.loglog(df["N"], df[f"{alg}_s"], marker="o", label=alg)
plt.title("Quadratic Sorting (Θ(n²)) — Log–log scale (N & time)")
plt.xlabel("N (input size, log scale)")
plt.ylabel("Time (seconds, log scale)")
plt.grid(True, which="both")
plt.legend()
plt.tight_layout()
plt.show()
