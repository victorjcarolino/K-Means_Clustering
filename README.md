**<h1>K-Means</h1>**

**<h2>Execution Data</h2>**
https://docs.google.com/spreadsheets/d/1ZcLFo0Y49qIlUCRqSpkb2Dtbx0JHtt5JW1wI-EmIYKQ/edit?usp=sharing

**<h2>Description</h2>**

**Kmeans-Serial**
- Base code provided and referenced from https://github.com/marcoscastro/kmeans
- Keeps track of: 
    - points + what cluster they are in
    - clusters + what points are in them
- Does the summation and mean of the cluster means at the end of each point association

**Better-Kmeans-Serial**
- Updated code from the aforementioned reference
- Keeps track of:
    - points + what cluster they are in
    - clusters + the intermediate sums from point addition and removal
- Does the summation of points to the mean as points are iterated 
    - Mean is computed sequentially at the end

**Kmeans-Parallel**
- Parallelized code from the Better-Kmeans-Serial
- Parallelization is done over the points rather than over the clusters due to a greater chance of sufficient parallel slack
- Data contention is handled through the use of thread local storage
- Keeps track of:
    - points + what cluster they are in
    - clusters + the intermediate sums from point addition and removal
    - thread local storage handles the summation of point additions and the number of points added to a cluster as well as the cluster switches made by points
- The thread local storage resolves the intermediate sums to the appropriate cluster and resolves the addition or removal of points to ensure proper calculation of the mean

**<h2>Datasets</h2>**
- Apple Quality
    - K: 8
    - Total Points: 3999
    - Total Features: 7
- Beans
    - K: 17
    - Total Points: 13611
    - Total Features: 16
- Big One
    - K: 32
    - Total Points: 100000
    - Total Features: 3
- Birch
    - K: 100
    - Total Points: 87996
    - Total Features: 2
- Gaussian Distribution
    - K: 9
    - Total Points: 10125
    - Total Features: 15
- HW Set 1
    - K: 2
    - Total Points: 7
    - Total Features: 2 
- HW Set 2
    - K: 3
    - Total Points: 150
    - Total Features: 4
- Pulsar
    - K: 2
    - Total Points: 17898
    - Total Features: 8

**<h2>Running</h2>**
- To execute the program:
    - run: sh run.sh datasets/[dataset-name].txt [output-subdirectory]
    - the outputs of the execution will be found in:
        - serial: outputs/[output-subdirectory]/kmeans-serial.txt
        - better serial: outputs/[output-subdirectory]/better-kmeans-serial.txt 
        - parallel: outputs/[output-subdirectory]/kmeans-parallel.txt
# Parallel K-means: optimization and thread-local aggregation

A C++ study comparing a provided serial baseline, an optimized serial implementation, and an Intel oneTBB parallel implementation. The parallel version distributes point assignment across threads and reduces thread-local cluster updates.

## Results and interpretation

Historical timings below are from the committed `outputs/` files, in microseconds. These are individual recorded runs, not repeated-run medians or claims about the revised code. Hardware, thread count, and toolchain were not recorded alongside these files. Printed centroids match across all three implementations on all eight recorded datasets; this is not a general correctness proof.

| Dataset | Baseline | Optimized serial | Parallel | Baseline / parallel | Optimized / parallel |
|---|---:|---:|---:|---:|---:|
| Apple quality | 35,872 | 8,768 | 6,186 | 5.80× | 1.42× |
| Beans | 1,629,905 | 408,037 | 124,109 | 13.13× | 3.29× |
| Big one | 2,184,938 | 733,230 | 183,837 | 11.89× | 3.99× |
| Birch | 1,912,590 | 1,166,905 | 611,572 | 3.13× | 1.91× |
| Gaussian distribution | 76,342 | 19,310 | 5,384 | 14.18× | 3.59× |
| HW set 1 | 9 | 7 | 354 | 0.03× | 0.02× |
| HW set 2 | 90 | 27 | 457 | 0.20× | 0.06× |
| Pulsar | 216,142 | 9,706 | 4,202 | 51.44× | 2.31× |

**51.44× is the combined improvement over the provided baseline, not the gain from parallelization alone.** Relative to optimized serial, the recorded Pulsar improvement is 2.31×. Tiny inputs are slower in parallel because scheduling overhead dominates.

## Build, run, and check

Requirements: C++17 compiler, make, Intel oneTBB, pkg-config, and Python 3 for tests. For example, install `libtbb-dev pkg-config` on Debian/Ubuntu or `tbb pkg-config` with Homebrew.

```sh
make
make test
sh run.sh datasets/pulsar.txt results/pulsar
```

Run from the repository root. The runner writes to the supplied output directory without modifying source or the Makefile. Override `TBB_CFLAGS` and `TBB_LIBS` if pkg-config cannot locate a custom installation. Do not overwrite historical `outputs/` when measuring changes.

The header is `points dimensions clusters max_iterations has_name`, followed by feature rows and an optional name per row. Initialization uses deterministic seeds within a given C library; random sequences may differ between platforms. Empty clusters retain their previous centroid.

## Attribution and scope

Original serial algorithm: [marcoscastro/kmeans](https://github.com/marcoscastro/kmeans). This repository explores incremental sums and thread-local parallel aggregation on top of that baseline. It is an educational performance project, not a production clustering library.

For new measurements, record the commit, CPU, core/thread limits, compiler flags, oneTBB version, dataset, initialization, warmups, repetitions, and median/dispersion. Check output equivalence before comparing timing.

## Original project notes

Historical notes below are retained for context. Use the build/run commands above for the current runner; its second argument is now the complete output directory.
