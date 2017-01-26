#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 
#define NUM_THREADS 6
#define NUM_STEPS 50000.00

#define A_location 20.34
#define B_location 50.12
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
int num_threads = 0;
int num_threads_depth_limit = 0;
int num_threads_thresh = 0;

long double STEP_SIZE;
long double THRESHHOLD = 20;
// long double MID_DEPTH = 2;
long double MAX_DEPTH = 7;//going from depth 10 to depth 8 reduces time by 2 orders of mag
// double function = 10.0;
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
    result += x*x*(sin(10*x)) + 100.00*x;
    // result += (cos(x) * X1);
    // result += (x*x * X2);
    // result += (x*x*x * X3);
    // result += (x*x*x*x * X4);
    // result += (x*x*x*x*x * X5);
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
        pthread_mutex_lock(&lock);
        num_threads = num_threads + 1;
        pthread_mutex_unlock(&lock);

// create some local vars
    long double* area = malloc(sizeof(long double));
    //long double area = 0;

    long double a = ((bounds*)bound)->a;
    long double b = ((bounds*)bound)->b;
    long double ab = ((b+a)/2);

    long double fa = fabsl(function(a));
    long double fb = fabsl(function(b));
    long double fab = fabsl(function(ab));
    long double favg = (fa+fb+fab)/3;
    
    int level = ((bounds*)bound)->level;
    //bool a = true;

// if the value of the functions at the bounds is more than the threashold spilt it in half
    if(((fabsl(fa-favg)>THRESHHOLD) || (fabsl(fb - favg) > THRESHHOLD) || (fabsl(fab - favg) > THRESHHOLD) ) && (level < MAX_DEPTH)){
        printf("Creating new threads\n");

// create new bounds
        bounds left, right;
        left.a = a;
        left.b = ((b+a))/2;
        left.level = ((bounds*)bound)->level + 1;
        right.a = ((b+a))/2;
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

// //count the number of threads
//         pthread_mutex_lock(&lock);
//         num_threads = num_threads + 2;
//         pthread_mutex_unlock(&lock);

// join threads
        void* left_a;
        void* right_a;

        rcl = pthread_join(left_t, &left_a);
        double long left_area = *(long double*)left_a;
        //free(left_a);
        printf("left area: %Lf\n", left_area);

        rcr = pthread_join(right_t, &right_a);
        double long right_area = *(long double*)right_a;
        //free(right_a);
        printf("right area: %Lf\n", right_area);

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
        *area = ((fabsl(b - a)/2)/3)*(fa + 4*fab + fb);

    }
//  output area
    printf("Area: %Lf\n", *area); 
    //return (void*)area;
    pthread_exit(area);
}

int main(int argc, const char* argv[]){
    printf("hello\n");

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

// create top thread vars
    pthread_t top_t;
    int rc;
    bounds top_bounds;
    top_bounds.a = A_location;
    top_bounds.b = B_location;
    top_bounds.level = 0;

// start clock
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
    clock_t diff = clock() - start;

// output result
    printf("---RESULTS----\n");
    // printf("Fucntion: (%f)x^5 (%f)x^4 (%f)x^3 (%f)x^2 (%f)x^1 (%f) \n", X5, X4, X3, X2, X1, C);
    printf("Fucntion: (%f)x^3 (%f)x^2 (%f)x^1 (%f) \n", X3, X2, X1, C);
    printf("Threshold: %Lf\n", THRESHHOLD);
    printf("Depth: %Lf\n", MAX_DEPTH);
    long double result = *(long double*)total_area;
    free(total_area);
    printf("Result: %Lf\n", result);
    long double adiff = fabsl(result - reS);
    printf("Differance: %Lf\n", adiff);
    long double perc = (adiff/result)*100;
    printf("Percentage: %Lf\n", perc);



// output time
    long double msec = diff * 1000.00 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds \n", (int)msec/1000, (int)msec%1000);
    printf("Time taken %Lf Miliseconds\n", msec);

// output number of threads
    printf("Total number of threads: %d \n", num_threads);
    printf("Number of max depth threads: %d \n", num_threads_depth_limit);
    printf("Number of threshold threads: %d \n", num_threads_thresh);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    
    return 0;
}



