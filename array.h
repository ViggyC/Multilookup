#ifndef ARRAY_H
#define ARRAY_H

#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/sem.h>
#include<pthread.h>
#include<stdlib.h>


#define ARRAY_SIZE 8 
#define MAX_NAME_LENGTH 255

//shared array
typedef struct {
    char* array[ARRAY_SIZE]; //array of string                 
    int counter; //counter is current position in buffer
    int operations;
    
    int num_consumed;
    int num_produced; //consumers need to know how many items have been produced, producers update this
    /*synchronization tools*/

    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
                           
} stack;

int  array_init(stack *s);                  // init the stack
int  array_put(stack *s, char *hostname);     // place element on the top of the stack
int  array_get (stack *s, char **hostname);    // remove element from the top of the stack
//int  array_get (stack *s, char *hostname);
void array_free(stack *s);  


//helper functions
int  array_position(stack *s);                // free the stack's resources
//void array_print(stack *s);

#endif