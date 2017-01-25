#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM_THREADS 6
#define NUM_STEPS 50000.00

#define A_location 10.34
#define B_location 20.12
#define C 1
#define X1 3
#define X2 -1
#define X3 4
#define X4 2
#define X5 0


pthread_mutex_t lock;
pthread_mutex_t lock1;
long double sum = 0;
long double STEP_SIZE;
//double function = 10.0;
int* a;
int* b;

long double function (long double x){
    long double result = 0.00;
    result += C;
    result += (x * X1);
    result += (x*x * X2);
    result += (x*x*x * X3);
    result += (x*x*x*x * X4);
    result += (x*x*x*x*x * X5);
    return result;
}

void *intergrate(void *index){
    //pthread_mutex_lock(&lock1);

    int in = (int)index;
    long double a = A_location + ((long double)(in)*STEP_SIZE);
    long double b = a + STEP_SIZE;

    // if(in < 5){
    //     printf("In: %d A: %Lf B: %Lf\n", in,a,b);
    // }
    // if(in > NUM_STEPS - 5){
    //     printf("In: %d A: %Lf B: %Lf\n", in,a,b);
    // }

    //printf("Hello thread: %d : %Lf to %Lf\n", in, a, b);
    long double area =   ( function(a) + ( function(b) - function(a) )*0.5  )* STEP_SIZE;

    pthread_mutex_lock(&lock);
    //printf("thread %d Adding Sum:", in);
    sum = sum + area;
    //printf(" %Lf\n", sum);
    pthread_mutex_unlock(&lock);

    //pthread_mutex_unlock(&lock1);    
    pthread_exit(NULL);
}

int main(int argc, const char* argv[]){
    printf("hello\n");
    STEP_SIZE = (B_location-A_location)/NUM_STEPS;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\nlock mutex init failed\n");
        return 1;
    }
    //lock1 just ensures that a thread is allows to compeltly execute before another is created, so there is no print errors
    if (pthread_mutex_init(&lock1, NULL) != 0)
    {
        printf("\nlock1 mutex init failed\n");
        return 1;
    }

    pthread_t threads[(int)NUM_STEPS];
    int rc, t;

    printf("Creating %f threads.\n", NUM_STEPS);
    clock_t start = clock(), diff;
    for(t=0; t<(int)NUM_STEPS; t++){
        //pthread_mutex_lock(&lock1);
        // printf("Creating thread %d.\n", t);
        // printf(".");

        rc = pthread_create(&threads[t], NULL, intergrate, (void*)t);        
        if(rc){
            printf("ERROR - return code from pthread_create: %d\n", rc);
            exit(-1);
        }
        //pthread_mutex_unlock(&lock1);
    }
    printf("Finished Creating threads\n");

    printf("Joining threads.\n");
    for(t=0; t<(int)NUM_STEPS; t++){
        //printf(".");
        //printf("Joining thread %d.\n", t);
        rc = pthread_join(threads[t], NULL);
        if(rc){
            printf("ERROR - return code from pthread_join: %d\n", rc);
            exit(-1);
        }
    }
    diff = clock() - start;
    printf("Finished Joining threads\n");

    printf("Result: %Lf\n", sum);

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);

    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock1);

    return 0;
}



