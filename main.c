#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 

#define NUM_THREADS 4

long double NUM_STEPS = 10000.00;
long double A_location = 0;
long double B_location = 1;
long double STEP_SIZE = 0;
long double A_CURR = 0;
long double AREA_TOTAL= 0;

long double function (long double x){
    long double result = 4.00L/(1.00L+(x*x));
    return result;
}

typedef struct {
    //Or whatever information that you need
    long double a;
    long double b;
    int index;
} bounds;

void *intergrate(void *bound){
// create some local vars
    int loops = 0;
    long double* area = malloc(sizeof(long double));

    long double A = ((bounds*)bound)->a;
    long double B = ((bounds*)bound)->b;
    int index = ((bounds*)bound)->index;

    long double step_size = (B-A)/NUM_STEPS;

    long double a = A;
    long double b = a+step_size;
    long double ab = ((b+a)/2);

    long double fa;
    long double fb;
    long double fab;

    while(a < B){
    // calculate new values
        fa = fabsl(function(a));
        fb = fabsl(function(b));
        fab = fabsl(function(ab));

//simposons rule ( first order )
        // area = h/3(fa + 4fab + fb);
        *area += ((step_size/2)/3)*(fa + 4*fab + fb);
 
// iterate area
        a = b;
        b = a + step_size;
        ab = ((b+a)/2);
        loops++;
    }
    // output area
    printf("Thread: %d Loops: %d Area: %Lf\n",index, loops, *area);
    pthread_exit((void*)area);
}

int main(int argc, const char* argv[]){
    printf("hello\n");

// create thread vars
    pthread_t threads[NUM_THREADS];
    int rc, t;
    bounds thread_bounds[NUM_THREADS];
    STEP_SIZE = (B_location - A_location)/NUM_THREADS;

// start clock
    clock_t start = clock();
    printf("Creating threads.\n");
// create first thread
    for(t = 0; t<NUM_THREADS; t++){
    // create bounds for curr thread
        thread_bounds[t].a = A_CURR;
        A_CURR = A_CURR + STEP_SIZE;
        thread_bounds[t].b = A_CURR;
        thread_bounds[t].index = t;
    // create thread
        rc = pthread_create(&threads[t], NULL, intergrate, (void*)&thread_bounds[t]);        
        if(rc){
            printf("ERROR - return code from pthread_create: %d ERROR: %d\n", t, rc);
            exit(-1);
        }
        printf("Finished creating thread %d\n", t);
    }

// join the threads
    void* temp_area;
    for(t = 0; t<NUM_THREADS; t++){
    // join thread
        rc = pthread_join(threads[t], &temp_area);
        if(rc){
            printf("ERROR - return code from pthread_join: %d ERROR: %d\n", t, rc);
            exit(-1);
        }
        printf("Finished Joining thread %d: area: %Lf\n",t, *(long double*)temp_area);
        AREA_TOTAL += *(long double*)temp_area;
        free (temp_area);
    }
    printf("Finished Joining threads\n");
// stop the cock
    clock_t diff = clock() - start;

// output result
    printf("---RESULTS----\n");
    printf("Total number of threads: %d \n", NUM_THREADS);
    printf("Step Size: %Lf\n", STEP_SIZE);
    printf("Steps: %Lf\n", NUM_STEPS);
    printf("Result: %Lf\n", AREA_TOTAL);
   
// output time
    long double msec = diff * 1000.00 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds \n", (int)msec/1000, (int)msec%1000);
    printf("Time taken %Lf Miliseconds\n", msec);

    return 0;
}



