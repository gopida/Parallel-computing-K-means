#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <omp.h>
#include "timer.h"

/*
 * Global Variables 
 */
int k_value;
double **data;
double **centroids;
double **old_centroids;
int **cluster_indexes;
int *centroids_index;
int rows, cols;
int *cluster_count;
double sse;

/*
 * Threads 
 */
int thread_count;
/*
 * Function declarations
 */
 
void parseStringFile(FILE *file, char ***holder);
void parseNumFile(FILE *file, double ***data);
int getRandomNumber(int lower, int upper);
void run_clustering();
double calc_eu_dist(double *vector1, double *vector2);
double vector_diff(double *vector1, double *vector2);
int find_min(double *vector);
void calc_centroids();
double* calc_mean(double *data , double *centroid, int k_count);
void create_copy(double **src, double ***dest);
double get_centroid_change(double **old_centroids, double **new_centroids);
void sum_squared_errors(); 

int main(int argv, char **argc){
    FILE *categories = fopen("credit_card_headers.csv","r");
    FILE *names = fopen("credit_card_cusid.csv","r");
    FILE *ratings = fopen("credit_card_data_1000.csv","r");

    if(categories == NULL || names == NULL || ratings == NULL){
        perror("File not found ");
        exit(EXIT_FAILURE);
    }
    
    char **features;
    char **cities;

    int i,j;
    double start, finish;
    int tolerance = 1;
    
    parseStringFile(categories, &features);
    parseStringFile(names, &cities);
    parseNumFile(ratings, &data);
    
    /*
     * memory allocations
     */
    cluster_indexes = (int **) malloc(sizeof(int *) * rows);
    
    /*
     * Thread - declaration and definitions
     */
    thread_count = strtol(argc[1], NULL, 10);
    
    GET_TIME(start);
    for(k_value=1;k_value<=10;k_value++){
        
    /*
     * choose k random datapoints from the dataset as centroid. 
     */
        sse = 0;
        centroids = malloc(sizeof(double *) * k_value);
        centroids_index = malloc(sizeof(int *) * k_value);
        old_centroids = malloc(sizeof(k_value));
        
        for(i=0;i<k_value;i++){
            
            centroids_index[i] = getRandomNumber(0, rows);
            centroids[i] = malloc(sizeof(double *) * cols);
            
            for(j=0;j<cols;j++){
                centroids[i][j] = data[centroids_index[i]][j];
            }
        }
        
        do{
            /* 
             * parallelization runs inside the method
             */
            run_clustering();

            create_copy(centroids, &old_centroids);
            
            /* 
             * parallelization runs inside the method
             */
            calc_centroids();

        }while(get_centroid_change(old_centroids, centroids) > tolerance);
       printf("\n When K = %d, \n", k_value);
        printf(" Centroids of clusters\n");
        for(i=0;i<k_value;i++){
            for(j=0;j<cols;j++){
                printf(" %0.0f ", centroids[i][j]);
            }
            printf("\n");
        }
        sum_squared_errors();
        printf("\n SSE = %f\n\n******************** \n", sse);
    }
    GET_TIME(finish);
    printf("\n The elapsed time is %e seconds\n", (finish - start));
    free( data );
    free( centroids );
    free( old_centroids );
    free( cluster_indexes );
    free( centroids_index );
    free( cluster_count );
    free( features );
    free( cities );
    free( categories );
    free( names );
    free( ratings );
    return 0;
}

void parseStringFile(FILE *file, char ***holder){
    
    char line[255];
    int index = 0;
    char **f_holder = malloc(sizeof(char *) * 1);
    while(fgets(line, 255 , file)) {
        int len = strlen(line);
        f_holder =  realloc(f_holder, sizeof(char *) * ++index);
        f_holder[index-1] = malloc(sizeof(char *) * len);
        strcpy(f_holder[index-1], line);
    }
    *holder = f_holder;
}

void parseNumFile(FILE *file, double ***data){
    char line[255];
    char delim[] = ",";

    int m = 0,n;
    double **f_data = (double **) malloc(sizeof(double *) * 1);
    char *pt;
   while(fgets(line, 255 , file)) {
        char *ptr = strtok(line, delim);
        ++m;
        f_data =  (double **) realloc(f_data, sizeof(double *) * m);
        f_data[m-1] = (double *) malloc(sizeof(double *) * 1);
        n = 0;
        while(ptr != NULL){
            ++n;
            f_data[m-1] = (double *) realloc(f_data[m-1], sizeof(double *) * n);
            f_data[m-1][n-1] = strtod(ptr, &pt);
            ptr = strtok(NULL, delim);
        }
    }
    
    rows = m;
    cols = n;
    *data = f_data;
}

int getRandomNumber(int lower, int upper){
    return (rand() % (upper - lower + 1)) + lower; 
}

void run_clustering(){
    
    int i,k;
    double *diff;
    int min;
#  pragma omp parallel for num_threads(thread_count)  \
        default(none) private(i, k, diff, min)  shared(cluster_indexes, data, centroids,rows, k_value)
    for(i=0;i<rows;i++){
        diff = malloc(sizeof(double) * k_value);
        cluster_indexes[i] = malloc(sizeof(int) * 2);
        for(k=0;k<k_value;k++){
            diff[k] = calc_eu_dist(data[i],centroids[k]);
        }
        min = find_min(diff);
        cluster_indexes[i][0] = i;
        cluster_indexes[i][1] = min;
    }
}

double calc_eu_dist(double *vector1, double *vector2){
    
    double dist = 0;
    double diff = 0;
    int i;
    for(i=0;i<cols;i++){
        diff = fabs(vector1[i]-vector2[i]);
        dist += pow(diff, 2);
    }
    return sqrt(dist);
}

double vector_diff(double *vector1, double *vector2){
    
    double diff = 0;
    int i;
    for(i=0;i<cols;i++){
        diff = diff + fabs(vector1[i]-vector2[i]);
    }
    return diff;
}

int find_min(double *vector){
    int k;
    double temp = 9007199254740992; //largest value of double
    int min;
    for(k=0;k<k_value;k++){
        if(vector[k]<temp){
            temp = vector[k];
            min = k;
        }
    }
    return min;
}

double* calc_mean(double *data , double *centroid, int k_count){

    int j;
    for(j=0;j<cols;j++){
        centroid[j] = (centroid[j]*k_count + data[j]) / (k_count+1);
    }
    return centroid;
}

void create_copy(double **src, double ***dest){
    int j,k;
    double **temp = malloc(sizeof(double*)*k_value);
    for(k=0;k<k_value;k++){
        temp[k] = malloc(sizeof(double*)*cols);
        for(j=0;j<cols;j++){
            temp[k][j] = src[k][j];
        }
    }
    *dest = temp;
}

double get_centroid_change(double **old_centroids, double **new_centroids){
    int k;
    double diff = 0;
    for(k=0;k<k_value;k++){
        diff = diff + vector_diff(old_centroids[k],new_centroids[k]);
    }
    return diff;
}

void calc_centroids(){

    int i,k,k_count;
    cluster_count = malloc(sizeof(int) * k_value);
    memset(cluster_count, 0, sizeof(*cluster_count));
    for(i=0;i<rows;i++){
        k = cluster_indexes[i][1];
        k_count = ++cluster_count[k];
        if(k_count == 1){
            centroids[k] = data[i];
        }else{
            centroids[k] = calc_mean(data[i], centroids[k], k_count);
        }
    }
}

void sum_squared_errors(){
    
    int i;
    double errors;
#  pragma omp parallel for num_threads(thread_count)  reduction(+:sse)\
        default(none) private(i, errors)  shared(cluster_indexes, centroids, data, rows)
    for(i=0;i<rows;i++){

        errors = calc_eu_dist(data[i],centroids[cluster_indexes[i][1]]);
        
        sse += (errors/rows);

    }
}
