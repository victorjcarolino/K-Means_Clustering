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

