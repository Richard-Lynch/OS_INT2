#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 

long double NUM_THREADS = 6.00L;
long double NUM_STEPS = 50000.00L;
long double STEP_SIZE = 0.00L;

long double A_location = -1.00L;
long double B_location = 1.00L;
long double reS = 3.14159265359L;
long double THRESHHOLD = 0.0005L;
long double MAX_DEPTH = 10.00L;//going from depth 10 to depth 8 reduces time by 2 orders of mag


pthread_mutex_t num_threads_lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
int num_threads = 0;
int num_threads_depth_limit = 0;
int num_threads_thresh = 0;

int* a;
int* b;

long double function (long double x){
    long double result = 2.00L*sqrt(1.00L-(x*x));
    // long double result = 4.00L*(1.00L/(x));
    return result;
}

typedef struct {
    //Or whatever information that you need
    long double a;
    long double b;
    int level;
} bounds;

void *intergrate(void *bound){
//count the number of threads
    pthread_mutex_lock(&num_threads_lock);
    num_threads = num_threads + 1;
    pthread_mutex_unlock(&num_threads_lock);

// create some local vars
    long double* area = malloc(sizeof(long double));

    long double a = ((bounds*)bound)->a;
    long double b = ((bounds*)bound)->b;
    long double ab = ((b+a)/2.00L);

    long double fa = fabsl(function(a));
    long double fb = fabsl(function(b));
    long double fab = fabsl(function(ab));
    long double favg = (fa+fb+fab)/3.00L;
    
    int level = ((bounds*)bound)->level;
    //bool a = true;

// if the value of the functions at the bounds is more than the threashold spilt it in ha.10lf
    if(((fabsl(fa-favg)>THRESHHOLD) || (fabsl(fb - favg) > THRESHHOLD) || (fabsl(fab - favg) > THRESHHOLD) ) && (level < MAX_DEPTH)){
        printf("Creating new threads\n");

// create new bounds
        bounds left, right;
        left.a = a;
        left.b = ((b+a))/2.00L;
        left.level = ((bounds*)bound)->level + 1;
        right.a = ((b+a))/2.00L;
        right.b = b;
        right.level = ((bounds*)bound)->level + 1;

// create new threads
        pthread_t left_t;
        pthread_t right_t;
        int rcl, rcr;

// create threads recursivly
        rcl = pthread_create(&left_t, NULL, intergrate, (void*)&left);
        rcr = pthread_create(&right_t, NULL, intergrate, (void*)&right);      
        if(rcl){
            printf("ERROR - return code from left pthread_create: %d\n", rcl);
            exit(-1);
        }
        if(rcr){
            printf("ERROR - return code from right pthread_create: %d\n", rcr);
            exit(-1);
        }

// join threads
        void* left_a;
        void* right_a;

        rcl = pthread_join(left_t, &left_a);
        double long left_area = *(long double*)left_a;
        //free(left_a);
        printf("left area: %.10Lf\n", left_area);

        rcr = pthread_join(right_t, &right_a);
        long double right_area = *(long double*)right_a;
        //free(right_a);
        printf("right area: %.10Lf\n", right_area);

        if(rcl){
            printf("ERROR - return code from left pthread_join: %d\n", rcl);
            exit(-1);
        }
        if(rcr){
            printf("ERROR - return code from right pthread_join: %d\n", rcr);
            exit(-1);
        }

// calculate area
        //if()
        *area = *(long double*)left_a + *(long double*)right_a;

        free(left_a);
        free(right_a);

    }else{
//check if the thread is being split because of level
        if(level >= MAX_DEPTH){
            pthread_mutex_lock(&lock1);
            num_threads_depth_limit = num_threads_depth_limit + 1;
            pthread_mutex_unlock(&lock1);
        }else{
            pthread_mutex_lock(&lock2);
            num_threads_thresh = num_threads_thresh + 1;
            pthread_mutex_unlock(&lock2);
        }

// otherwise calc the area using trap rule
    //trap rule ( first order )
        //*area = (fa + (fabsl(fb - fa)) * 0.5 ) * (fabsl(b - a));
        
    //trap rule ( second order )
        //*area = ((fa + (fabsl(fab - fa)) * 0.5 ) * (fabsl((b - a)/2))) +  ((fb + (fabsl(fab - fb)) * 0.5 ) * (fabsl((b - a)/2)));
        
    //simposons rule ( first order )
        // area = h/3(fa + 4fab + fb);
        long double two = 2.0000L;
        long double three = 3.0000L;
        long double four = 4.0000L;
        *area = ((fabsl(b - a)/two)/three)*(fa + four*fab + fb);

    }
//  output area
    printf("Area: %.10Lf\n", *area); 
    //return (void*)area;
    pthread_exit(area);
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
// create top thread vars
    pthread_t top_t;
    int rc;
    bounds top_bounds;
    top_bounds.a = A_location;
    top_bounds.b = B_location;
    top_bounds.level = 0;

// start clock
    // mach_timespec_t ti_start = {0,0};
    // mach_timespec_t ti_end = {0,0};
    // mach_timespec_t ti_elap = {0,0};
    // uint64_t test = mach_absolute_time(void);
    // clock_get_time(, &ti_start);
    clock_t start = clock();
    printf("Creating top thread.\n");
// create first thread
    rc = pthread_create(&top_t, NULL, intergrate, (void*)&top_bounds);        
    if(rc){
        printf("ERROR - return code from top pthread_create: %d\n", rc);
        exit(-1);
    }
    printf("Finished creating thread\n");

// join the threads
    void* total_area;
    rc = pthread_join(top_t, &total_area);
    if(rc){
        printf("ERROR - return code from pthread_join: %d\n", rc);
        exit(-1);
    }
    printf("Finished Joining threads\n");
// stop the cock
    clock_t tdiff = clock() - start;
    // clock_get_time(CLOCK_THREAD_CPUTIME_ID, &ti_end);
    // ti_elap = diff(ti_start, ti_end);


// output result
    printf("---RESULTS----\n");
    printf("Fucntion: 2! x SQRT(1-x^2)\n");
    printf("Actual: %.10Lf\n",reS);
    printf("Threshold: %.10Lf\n", THRESHHOLD);
    printf("Depth: %.10Lf\n", MAX_DEPTH);
    long double result = *(long double*)total_area;
    free(total_area);
    printf("Result: %.10Lf\n", result);
    long double adiff = fabsl(result - reS);
    printf("Differance: %.10Lf\n", adiff);
    long double perc = (adiff/result)*100.00L;
    printf("Percentage: %.10Lf\n", perc);



// output time
    long double msec = tdiff * 1000.00L / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds \n", (int)msec/1000, (int)msec%1000);
    printf("Time taken %.10Lf Miliseconds\n", msec);

    //printf("REAL Time taken %d seconds %d milliseconds %d nanoseconds \n", ti_elap.tv_sec, (int)ti_elap.tv_nsec/1000, (int)ti_elap.tv_sec%1000);

// output number of threads
    printf("Total number of threads: %d \n", num_threads);
    printf("Number of max depth threads: %d \n", num_threads_depth_limit);
    printf("Number of threshold threads: %d \n", num_threads_thresh);
    pthread_mutex_destroy(&num_threads_lock);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    
    return 0;
}



