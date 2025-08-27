1.1 Understanding the code
Run the application with a configuration file (test.dat). The application will run a sequence of performance measurements for simulations on different discretizations/sizes of a 2D plate, printing out FLOP/s numbers. In addition, a file "heat.ppm" is dumped at the end, providing the solution as image (as portable pixmap file format). You can load it with command "xv". You can also convert it to PNG with "convert heat.ppm heat.png".

1. Show the initial performance data. The form of the graph was introduced in the meeting.

2. Explain how the FLOP/s metric is measured. Which floating point operations are taken into account?

1.2 Compiler Options
 Measure the performance achievable with combinations of the following options "-O1", "-O2", "-O3", and "-Ofast" and "-xhost" (change CFLAGS in Makefile for that). In addition, options "-ipo" and "-fno-alias" and "-xCORE-AVX512" are useful. You can also try to use compiler pragmas such as "ivdep" (look it up!). 

Find the combination of flags that gives you the best performance. Measure on the login node and on the compute nodes.

Use the Intel compiler.

Be able to answer the following questions:

What is the meaning of -ipo?
What is the meaning of -fno-alias?
What is the meaning of "ivdep"? 
What is the meaning of "-xCORE-AVX512"?
The Intel compiler provides reports when using "opt-report" option. Remember that only relax_jacobi.c is relevant for the overall performance. What does it tell you, and what does it mean?

Use the following options to trigger opt-report: -qopt-report-annotate -qopt-report-phase=vec,loop. Copy the output for relax_jacobi and explain what it tells you. 

Be able to answer the following questions:

Is the code vectorized by the compiler? 
What does it mean by -qopt-zmm-usage?
What is the performance result of these options. Present a graph as outlined in the meeting.
Report on any performance variability you found as presented in the SuperMUC NG presentation.
1.3 Batch Processing
Run the code as batch job. Does the performance differ to a run on the login node? Give a second graph. Can you think about any reason?

1.4 Manual optimization
In this assignment, you should come up with changes of heat that improve sequential performance. For this, check the following points for ways to improve the code:

access pattern into the matrix
calculation of residuum
avoid copy operation
Provide a graphs to understand the effect of your different improvements on the execution time of the application.  
 
Deliverables
Bring two Graphs (as one PDF with readable fonts and colors when projected) to Frauenchiemsee and upload them here before the meeting. The filename (group1_perf.pdf or group1_batch_script.txt) and the title should include your group number.
Graph 1: Compare the initial performance data vs different compiler flags. Be able to argue about the results. Be able to show and explain what the optimization report tells you.
Graph 2: Select the best combination and plot the performance of the application run on the login node and on a batch node. Please use a box plot to show the amount of variation. Do the multiple runs in batch on the same node. Thus, use a batch script where you start the code multiple times. 
Provide an additional graph with the MFlop rate showing the  improvements achieved by your different manual modifications. 
Submit your batch script and the job output from the batch run as two files.
Submit everything as individual files. Not as an archive.



Assignment 2: Parallelization with MPI
Both dimensions of the domain should be distributed.
The real two-dimensional distribution (1x4, 2x2,4x1, ...) can be specified via command line, i.e., a.out 4 1 for 4x1, arguments.
Parallelize the coarsening, i.e., local coarsening in each rank.
The residual should be independent of the processor configuration and be computed in each Jacobi iteration.
Use MPI virtual topologies.
Parallelize Jacobi


Abalone:
Details on how to run Abalone on SuperMUC-NG
First, it is highly recommended to run Abalone as an interactive job (using salloc) and not as a batch job - this allows you to control the individual processes as well as the game startup. For testing, it is safe to run all processes on one node - this way all the default port settings should work. For testing the competitive mode, the following steps should work:

Step 1:
Open three shells on the SuperMUC-NG login nodes (one for each player and one for the referee)
Step 2:
On the shell for the referee, allocate two nodes (using salloc -N 2). Note down the job ID (called JID in the following) and determine the node names by running "srun hostname", which we will call host1 and host2. 
Step 3:
From one of the two other shells, start one player on host2 using srun with "srun -N1 -n X -w host2 --jobid JID player O -p 8000". X determines the number of MPI processes you want (make sure this is smaller or equal than the number of available cores) - if you are using OpenMP, specify 1. Note: only use the actual host name (e.g., i01r01c04s02) not the fully qualified host name with domain.
Step 4:
From the remaining shell start the second player on host 1 using srun with "srun -N1 -n X -w host1 --jobid JID player X -p 7000".
Step 5:
Kick of the game from the referee shell with "referee -p host2:8000 -p 7000".




Likwid Measurments:
The goal of this step is to understand the reasons for the performance behavior of the O2, compiler optimized, and manually optimized code. 

Analyze the performance of the application with respect to the memory and cache usage. Perform all final measurements for the original code with O2 and the best flag combination (-O3 -fno-alias -xhost) for test.dat in a batch job! Groups with odd numbers should use PAPI and Groups with even numbers should use Likwid.

2.1 Provide a diagram with the MFlop rate and the L2 and L3 missrates in percent of cache accesses for the application compiled with -O2 and a second for the application compiled wiht -O3 -fno-alias -xhost. Present also table with the number of L2 and L3 cache misses with the diagrams in thousands for the different problem sizes. 

Diagram with the PAPI measurements

2.2 Measure the MFlop rate with the hardware counters and compare the result with the number computed in the program.

2.3 Determine the processor's clock frequency with the help of the performance counters. 

2.4 Confirm, whether vectorization was used or not for the two differently compiled versions, with the help of the performance counters. 

2.5 After your measurements with PAPI or Likwid, investigate the performance of both versions with the help of vtune. Determine the overhead of running vtune data collector. Check the performance of both versions (O2, optimized) for 3200 resolution and 200 iterations. Be able to show your performance numbers and highlight interesting aspects.

Comparison of the compiler-optimized version and the manually optimized version

3.1 Use PAPI / LIKWID to compare the two versions for the different problem size in test.dat. Submit a diagram where you compare certain metrics for each problem size.

Comparison

3.2 There is a performance issue around sizes of powers of 2, e.g. np=1020, 1022, 1024, 1026. Can you explain it? How to get rid of that issue? 

Deliverables
Submit three graphs:

The two PAPI/LIKWID graphs for O2 and compiler optimized.
The comparison of the compiler optimized version and the manually optimized version. 
Be able to show your measurements with vtune. 


Parallelization of Jacobi with OpenMP:

ASSIGNMENT 4.1: Sequential performance
Measure execution time for your optimized code. Report the initial performance for all test configuration in test.dat. Run in batch. We want to make sure that all of you have the same base performance. Use -O3 -fno-alias -xhost.

Submit a copy of the output immediatly.


ASSIGNMENT 4: Parallelization with OpenMP
Manual parallelization of Jacobi with OpenMP

TASK 4.1: Parallelize the Jacobi algorithm with OpenMP.
Run the given problem sizes with 1, 2, 4, 8, 12, 16, 24, 32 and 48 threads. 
TASK 4.2: Optimize for NUMA according to first touch allocation policy. Any other optimization possibilities? Rerun with optimized code and compare to the results above.
TASK 4.3: Idenitify how to adjust the thread to core mappings and provide absolute performance and speedup comparisons between multiple assignments (at least scatter and compact). Use the KMP_AFFINITY environment variable. (https://www.intel.com/content/www/us/en/develop/documentation/oneapi-dpcpp-cpp-compiler-dev-guide-and-reference/top/optimization-and-programming/openmp-support/openmp-library-support/thread-affinity-interface.html)
Deliverable

Provide a speedup graph comparing with and without first touch for the two configurations (2000,6000). Without any special Affinity settings.  (filename: GroupN_ASS4.3_speedup.png)
Provide a speedup graph for the configurations (2000, 6000) with two curves for compact and scatter thread distribution with first touch. (filename: GroupN_ASS4.3_first_touch.png)
Run also measurements with vtune for your best OpenMP version with 3200 resolution. Compare the measurements to the vtune results with a single thread. Submit interesting graphs with annotations.(filenames: GroupN_ASS4.3_vtune...png)
Present the graphs in the afternoon session.




Parallelization of Heat with MPI:
Parallelization with MPI
Both dimensions of the domain should be distributed.
The real two-dimensional distribution (1x4, 2x2,4x1, ...) can be specified via command line, i.e., a.out 4 1 for 4x1, arguments.
Parallelize the coarsening, i.e., local coarsening in each rank.
The residual should be independent of the processor configuration and be computed in each Jacobi iteration.
Use MPI virtual topologies.
Try to overlap communication and computation with MPI non-blocking communication (start with a version using blocking communication).
Parallelize Jacobi
Start from the optimized code from the OpenMP assignment.
Performance measurements
Compare the execution time and the speedup for Jacobi for different processor configurations (1D-x, 1D-y, 2D). Speedup should be computed based on the execution time of the sequential version, not the one-rank parallel version. Use a maximum of 4 nodes (pick a suitable number of cores) in your configurations. Add a new node only, when all the physical cores on the already used nodes are busy. Otherwise, we loose efficiency. Do not use hyperthreading.
Compare the execution times for blocking and non-blocking communication.
Perform tests with the two problem sizes 2000 and 6000
Investigate hybrid parallelization as well.
Deliverables:
Submit a speedup graph for the two problem sizes 2000 and 6000 for the pure MPI version. 
Submit a graph comparing blocking vs non-blocking communication for problem size 6000. Again pure MPI only. X-axis are the best configurations for the blocking case. Labels should tell us your best configuration and look like (2x1, 4x4 ...). 