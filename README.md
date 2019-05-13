# Parallel-computing-K-means
Parallelization of K-Means Clustering Algorithm

# Code modification for different samples
Please replace the below file name accordingly for execution
FILE *ratings = fopen("credit_card_data_1000.csv","r");

# Compile and execute commands

gcc -g -Wall -o kmeancluster kmeancluster.c -lm

./kmeancluster



gcc -g -Wall -o kmeancluster_pthread kmeancluster_pthread.c -lm -lpthread

./kmeancluster_pthread 2

./kmeancluster_pthread 4

./kmeancluster_pthread 8

./kmeancluster_pthread 10



gcc -g -Wall -fopenmp -o kmeancluster_omp kmeancluster_omp.c -lm

./kmeancluster_omp 2

./kmeancluster_omp 4

./kmeancluster_omp 8

./kmeancluster_omp 10



# Output object

centroids[i][j] – final output of centroids.

centroids_index – membership of each samples to centroids.

sse – sum of squared errors.

# dataset

Uploaded along with this code
