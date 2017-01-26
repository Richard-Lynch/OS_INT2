#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 
#define NUM_THREADS 4
long double NUM_STEPS = 10000.00;
long double A_location = 20.34;
long double B_location = 50.12;
#define C 25000.00
#define X1 11.00
#define X2 -47.5
#define X3 1.00
#define X4 0.00
#define X5 0.00
// #define reS 430591.00
#define reS 104856.00


pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
//int num_threads = 0;
// int num_threads_depth_limit = 0;
// int num_threads_thresh = 0;


long double AREA_TOTAL = 0;
long double STEP_SIZE = 0;
long double A_CURR = 0;

long double THRESHHOLD = 20;
long double MAX_DEPTH = 7;//going from depth 10 to depth 8 reduces time by 2 orders of mag
int* a;
int* b;

long double function1 (long double x){
    long double result = 0.00;
    result += C;
    result += (cos(x) * X1);
    result += (x*x * X2);
    result += (x*x*x * X3);
    result += (x*x*x*x * X4);
    result += (x*x*x*x*x * X5);
    return result;
}

long double function (long double x){
    long double result = 0.00;
    result = x*x*x*(sin(10.00*x)) + 100.00*x;
    return result;
}



typedef struct {
    //Or whatever information that you need
    long double a;
    long double b;
    int level;
} bounds;

void *intergrate(void *bound){
// create some local vars
    int* loop = malloc(sizeof(int));
    long double area = 0;

    long double a = ((bounds*)bound)->a;
    long double b = ((bounds*)bound)->b;
    long double ab = ((b+a)/2);

    long double fa = fabsl(function(a));
    long double fb = fabsl(function(b));
    long double fab = fabsl(function(ab));
    //long double favg = (fa+fb+fab)/3;
    
    int level = ((bounds*)bound)->level;
    

while(a < B_location){
// calculate new values
    fa = fabsl(function(a));
    fb = fabsl(function(b));
    fab = fabsl(function(ab));

    //trap rule ( first order )
        //*area = (fa + (fabsl(fb - fa)) * 0.5 ) * (fabsl(b - a));
        
    //trap rule ( second order )
        //*area = ((fa + (fabsl(fab - fa)) * 0.5 ) * (fabsl((b - a)/2))) +  ((fb + (fabsl(fab - fb)) * 0.5 ) * (fabsl((b - a)/2)));
        
    //simposons rule ( first order )
        // area = h/3(fa + 4fab + fb);
        area = ((STEP_SIZE/2)/3)*(fa + 4*fab + fb);
// output area
        printf("Thread: %d Area: %Lf\n",level, area); //AREA_TOTAL,
        pthread_mutex_lock(&lock);
        AREA_TOTAL += area;
        pthread_mutex_unlock(&lock);
// iterate area
        pthread_mutex_lock(&lock1);
        a = A_CURR;
        A_CURR = A_CURR + STEP_SIZE;
        b = A_CURR;
        pthread_mutex_unlock(&lock1);
        ab = ((b+a)/2);
        (*loop)++;
}
    //return (void*)area;
    pthread_exit((void*)loop);
}

int main(int argc, const char* argv[]){
    printf("hello\n");

// calculate step size
    STEP_SIZE = (B_location - A_location)/NUM_STEPS;

// init mutex lock
    if (pthread_mutex_init(&lock, NULL) != 0)
        {
            printf("\nlock mutex init failed\n");
            return 1;
        }
    if (pthread_mutex_init(&lock1, NULL) != 0)
        {
            printf("\nlock1 mutex init failed\n");
            return 1;
        }
    if (pthread_mutex_init(&lock2, NULL) != 0)
        {
            printf("\nlock2 mutex init failed\n");
            return 1;
        }

// create thread vars
    pthread_t threads[NUM_THREADS];
    int rc, t;
    bounds thread_bounds[NUM_THREADS];
    A_CURR = A_location;

// start clock
    clock_t start = clock();
    printf("Creating threads.\n");
// create first thread
    for(t = 0; t<NUM_THREADS; t++){
    // create bounds for curr thread and iterate A_CURR
        pthread_mutex_lock(&lock1);//
        thread_bounds[t].a = A_CURR;
        A_CURR = A_CURR + STEP_SIZE;
        thread_bounds[t].b = A_CURR;
        pthread_mutex_unlock(&lock1);//
        thread_bounds[t].level = t;
    // create thread
        rc = pthread_create(&threads[t], NULL, intergrate, (void*)&thread_bounds[t]);        
        if(rc){
            printf("ERROR - return code from pthread_create: %d ERROR: %d\n", t, rc);
            exit(-1);
        }
        printf("Finished creating thread %d\n", t);
    }

// join the threads
    void* total_loops;
    for(t = 0; t<NUM_THREADS; t++){
    // join thread
        rc = pthread_join(threads[t], &total_loops);
        if(rc){
            printf("ERROR - return code from pthread_join: %d ERROR: %d\n", t, rc);
            exit(-1);
        }
        printf("Finished Joining thread %d: total loops: %d\n",t, *(int*)total_loops);
    }
    printf("Finished Joining threads\n");
// stop the cock
    clock_t diff = clock() - start;

// output result
    printf("---RESULTS----\n");
    pthread_mutex_lock(&lock);
    // printf("Fucntion: (%f)x^3 (%f)x^2 (%f)x^1 (%f) \n", X3, X2, X1, C);
    // printf("Threshold: %Lf\n", THRESHHOLD);
    printf("Total number of threads: %d \n", NUM_THREADS);
    printf("Step Size: %Lf\n", STEP_SIZE);
    printf("Steps: %Lf\n", NUM_STEPS);

    printf("Result: %Lf\n", AREA_TOTAL);
    long double adiff = fabsl(AREA_TOTAL - reS);
    printf("Differance: %Lf\n", adiff);
    long double perc = (adiff/AREA_TOTAL)*100;
    printf("Percentage: %Lf\n", perc);
    pthread_mutex_unlock(&lock);


// output time
    long double msec = diff * 1000.00 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds \n", (int)msec/1000, (int)msec%1000);
    printf("Time taken %Lf Miliseconds\n", msec);

// output number of threads
    
    // printf("Number of max depth threads: %d \n", num_threads_depth_limit);
    // printf("Number of threshold threads: %d \n", num_threads_thresh);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    
    return 0;
}



