#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 

int NUM_THREADS = 6.00L;
long double NUM_STEPS = 50000.00L;
long double STEP_SIZE = 0.00L;

long double reS = 3.14159265359L;
long double THRESHHOLD = 0.0001L;

pthread_mutex_t num_threads_lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t sum_lock;
pthread_mutex_t curr_x_lock;

long double curr_x = 1.00L;
long double sum = 0;

int num_threads = 0;
int num_threads_depth_limit = 0;
int num_threads_thresh = 0;

int* a;
int* b;

long double function (long double x){
    // long double result = 2.00L*sqrt(1.00L-(x*x));
    long double result = (1.00L/(x));
    return result;
}

typedef struct {
    //Or whatever information that you need
    long double x;
    long double limit;
    int index;
} bounds;

void *intergrate(void *bound){
//count the number of threads
    pthread_mutex_lock(&num_threads_lock);
    num_threads = num_threads + 1;
    pthread_mutex_unlock(&num_threads_lock);

// create some local vars
    long double oldSum = -1;

    long double x = ((bounds*)bound)->x;
    long double limit = ((bounds*)bound)->limit;

    long double fa;
    
    int* index = &((bounds*)bound)->index;

    int loop = 1;

    while(loop > 0){
// calculate new values
        fa = function(x);
// add the value to the sum
        pthread_mutex_lock(&sum_lock);
        sum += fa; 
        pthread_mutex_unlock(&sum_lock);

// check accuracy 
        if(fabsl(oldSum+sum/2)<limit){
            // if accuate then we're done
            loop = -1;
        }else{
            // otherwise, iterate to the next sum
            oldSum = sum;
            x = curr_x; //x = curr x
            if(x < 0){  //if neg
                curr_x = (-curr_x)+2;
            }else{      //if pos
                curr_x = -(curr_x+2);
            }
            pthread_mutex_unlock(&curr_x_lock);
        }
    }

    pthread_exit((void*)index);
}
// -----------------------------------

int main(int argc, const char* argv[]){
    printf("hello\n");

// init mutex lock
    if (pthread_mutex_init(&num_threads_lock, NULL) != 0)
        {
            printf("\nnum_threads_lock mutex init failed\n");
            return 1;
        }
    if (pthread_mutex_init(&sum_lock, NULL) != 0)
        {
            printf("\nsum_lock mutex init failed\n");
            return 1;
        }
    if (pthread_mutex_init(&curr_x_lock, NULL) != 0)
        {
            printf("\ncurr_x_lock init failed\n");
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
// create local vars
    pthread_t threads[NUM_THREADS];
    int rc,t;
    bounds t_init;

// start clock
    clock_t start = clock();
    printf("Creating top thread.\n");
// create first thread
    for(t = 0; t<NUM_THREADS; t++){

        pthread_mutex_lock(&curr_x_lock);
        t_init.x = curr_x;
        if(curr_x < 0){  //if neg
            curr_x = (-curr_x)+2;
        }else{      //if pos
            curr_x = -(curr_x+2);
        }
        pthread_mutex_unlock(&curr_x_lock);
        
        t_init.limit = THRESHHOLD;
        t_init.index = t;
        rc = pthread_create(&threads[t], NULL, intergrate, (void*)&t_init);        
        if(rc){
            printf("ERROR - return code from thread %d pthread_create: %d\n", t, rc);
            exit(-1);
        }
        printf("Finished creating thread\n");
    }
    

// join the threads
    void* index;
    for(t = 0; t<NUM_THREADS; t++){
            rc = pthread_join(threads[t], &index);
    if(rc){
        printf("ERROR - return code from pthread_join: %d\n", rc);
        exit(-1);
    }else{
        printf("Joined thread: %d\n", *(int*)index);
    }
    }
    printf("Finished Joining threads\n");
// stop the cock
    clock_t tdiff = clock() - start;



// output result
    printf("---RESULTS----\n");
    printf("Fucntion: Leibniz\n");
    printf("Actual: %.10Lf\n",reS);
    printf("Threshold: %.10Lf\n", THRESHHOLD);

    pthread_mutex_lock(&sum_lock);
    sum = sum*4.00L;
    printf("Result: %.10Lf\n", sum);
    long double adiff = fabsl(sum - reS);
    printf("Differance: %.10Lf\n", adiff);
    long double perc = (adiff/sum)*100.00L;
    pthread_mutex_unlock(&sum_lock);
    printf("Percentage: %.10Lf\n", perc);

// output time
    long double msec = tdiff * 1000.00L / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds \n", (int)msec/1000, (int)msec%1000);
    printf("Time taken %.10Lf Miliseconds\n", msec);

// output number of threads
    // printf("Total number of threads: %d \n", num_threads);
    // printf("Number of max depth threads: %d \n", num_threads_depth_limit);
    // printf("Number of threshold threads: %d \n", num_threads_thresh);
    pthread_mutex_destroy(&num_threads_lock);
    pthread_mutex_destroy(&sum_lock);
    pthread_mutex_destroy(&curr_x_lock);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    
    return 0;
}



