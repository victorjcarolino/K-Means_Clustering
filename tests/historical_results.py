"""Recompute the README ratios from retained historical outputs (not new timings)."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1] / "outputs"
for folder in sorted(path for path in root.iterdir() if path.is_dir()):
    timings, centers = [], []
    for name in ["kmeans-serial", "better-kmeans-serial", "kmeans-parallel"]:
        text = (folder / (name + ".txt")).read_text()
        timings.append(int(re.search(r"TOTAL EXECUTION TIME\s*=\s*(\d+)", text).group(1)))
        centers.append(re.findall(r"Cluster values:\s*([^\n]+)", text))
    assert centers[0] == centers[1] == centers[2], folder.name
    baseline, serial, parallel = timings
    print(f"{folder.name}: {baseline}, {serial}, {parallel} us; "
          f"{baseline/parallel:.2f}x baseline; {serial/parallel:.2f}x optimized; printed centroids match")
