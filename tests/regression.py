"""Deterministic regressions; no third-party Python dependencies."""
import math
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PROGRAMS = ["kmeans-serial", "better-kmeans-serial", "kmeans-parallel"]


def run(program, path):
    return subprocess.run([str(ROOT / "bin" / program), str(path)],
                          capture_output=True, text=True, timeout=60)


def centers(output):
    return [[float(v) for v in line.split()]
            for line in re.findall(r"Cluster values:\s*([^\n]+)", output)]


with tempfile.TemporaryDirectory() as folder:
    path = Path(folder) / "data.txt"
    for dimensions in [1, 4, 16]:
        path.write_text(f"3 {dimensions} 1 1 0\n" +
                        "\n".join(" ".join([str(x)] * dimensions) for x in [0, 3, 6]) + "\n")
        for program in PROGRAMS:
            result = run(program, path)
            assert result.returncode == 0, result.stderr
            assert centers(result.stdout) == [[3.0] * dimensions], (program, result.stdout)
    path.write_text("4 2 3 10 0\n1 1\n1 1\n1 1\n1 1\n")
    for program in PROGRAMS:
        result = run(program, path)
        assert result.returncode == 0, result.stderr
        assert centers(result.stdout) == [[1.0, 1.0]] * 3
    for invalid in ["", "2 1 0 1 0\n", "2 1 3 1 0\n", "2 1 1 0 0\n",
                    "2 1 1 1 0\n1\n", "1 1 1 1 0\nnan\n",
                    "1 1 1 1 1\n3\n"]:
        path.write_text(invalid)
        for program in PROGRAMS:
            assert run(program, path).returncode != 0, (program, invalid)
    for program in PROGRAMS:
        assert subprocess.run([str(ROOT / "bin" / program)], capture_output=True).returncode != 0

for dataset in sorted((ROOT / "datasets").glob("*.txt")):
    expected = None
    for program in PROGRAMS:
        result = run(program, dataset)
        assert result.returncode == 0, (program, dataset, result.stderr)
        actual = centers(result.stdout)
        assert actual and all(math.isfinite(x) for row in actual for x in row)
        if expected is None:
            expected = actual
        else:
            assert len(actual) == len(expected)
            for left, right in zip(actual, expected):
                assert len(left) == len(right)
                assert all(math.isclose(a, b, rel_tol=1e-4, abs_tol=1e-5)
                           for a, b in zip(left, right)), (program, dataset, left, right)
print("PASS: eight datasets, dimensions, iteration limit, empty clusters, invalid input")
