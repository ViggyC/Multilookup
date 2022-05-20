/* Author: Vignesh Chandrasekhar */
//collaborator: Sidhant Puntambekar
#include "array.h"
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/sem.h>
#include<pthread.h>
#include<stdlib.h>

//PART A
#define PRODUCER_THREADS 5
#define CONSUMER_THREADS 5
#define PRODUCE 10
#define CONSUME 10


//we have two pools of threads -> consumers and producers
/*This test should test the shared buffer created in array.c*/
/*This test will produce 100 times and the consumer should process all items*/



//task for threads are to consume and produce, and these are the arguments
typedef struct Task{
    //shared resources
    stack sharedBuffer;
    
    pthread_mutex_t count_produced; //locking mechanism when consumer tries to check producer progress
    pthread_mutex_t buffer_position; //locking mechanism when consumer tries to check
 
    
}Task;


//function that calls array_put
void* producer(void* args){
    Task *data = (Task *) args;
    //args should be the shareed buffer and a random string
    //array_put(args->buffer, args->randomString);
    int item;
    //lets produce NUM_ITEMS items 
    for(int i=0; i<PRODUCE; i++){
        //pthread_mutex_lock(&(data->put));
        item = rand()%100;
        char str[MAX_NAME_LENGTH];
        sprintf (str, "%d", item);
        array_put( &(data->sharedBuffer), str );

        //pthread_mutex_unlock(&(data->get))
    }

    return NULL;

}


void* consumer(void* args){

    Task *data = (Task *) args;
    //args should be the shareed buffer and the address to random string
    char *hostname = malloc(sizeof(char) *MAX_NAME_LENGTH); //type of this is character array = char *hostname
 
    //char *hostname[MAX_NAME_LENGTH];
    for(int i=0; i<CONSUME; i++){
        array_get( &(data->sharedBuffer), &hostname ); //&hostname = char **hostname
        
    }

    /*while(1){

        //this mutex needs to only allow one consumer to read num_produced so we can terminate
        pthread_mutex_lock(&(data->count_produced)); //locking num_produced
        pthread_mutex_lock(&(data->buffer_position)); //locking counter within shared buffer

        //int temp_numProduced = data->sharedBuffer.num_produced;
        //int temp_counter = data->sharedBuffer.counter;
        //pthread_mutex_lock(&(data->get));
        //we have to lock this check because consumer could access wrong value

        if(data->sharedBuffer.num_produced==NUM_ITEMS*PRODUCER_THREADS && data->sharedBuffer.counter<=0){
            printf("Done consuming\n");
            pthread_mutex_unlock(&(data->buffer_position));
            pthread_mutex_unlock(&(data->count_produced));
            return 0;
        }else{
            //array_get( &(data->sharedBuffer), &hostname );
            pthread_mutex_unlock(&(data->buffer_position));
            pthread_mutex_unlock(&(data->count_produced));
        }

        array_get( &(data->sharedBuffer), &hostname );

         

    }*/

    free(hostname);
    return NULL;

}


//this main function is a thread
int main(){

    Task thread_data;
    //stack buffer_test; //shared access
    int intialize = array_init(&thread_data.sharedBuffer);
  
    if(intialize ==0){
        printf("Initilization of buffer successful\n\n");
    }

    //now try to create one producer thread and one consumer thread
    
    pthread_t p[PRODUCER_THREADS];
    pthread_t c[CONSUMER_THREADS];

    pthread_mutex_init(&(thread_data.count_produced), 0);
    
    pthread_mutex_init(&(thread_data.buffer_position), 0);




    for(int i=0; i<PRODUCER_THREADS; i++){
        //int* a = malloc(sizeof(int));
        //*a = i;
        //thread_data.producer_num = *a;
        if(pthread_create(&p[i], NULL, producer, &thread_data) !=0){
            printf("Failed to create producer thread\n");
        }

    }

    for(int i=0; i<CONSUMER_THREADS; i++){
        //int* a = malloc(sizeof(int));
        //*a = i;
        //thread_data.producer_num = *a;
        if(pthread_create(&c[i], NULL, consumer, &thread_data) !=0){
            printf("Failed to create thread\n");
        }

    }

   
    for(int i=0; i<PRODUCER_THREADS; i++){
        
        if(pthread_join(p[i], NULL)!=0){
            printf("Failed to join producers\n");
        }

    }

    for(int i=0; i<CONSUMER_THREADS; i++){
        if(pthread_join(c[i], NULL)!=0){
            printf("Failed to join consumers\n");
        }

    }


    // char test[] = "Hello";
    // printf("Buffer counter is %d\n" , array_position(&thread_data.sharedBuffer));
    // //now put something
    // for(int i=0; i<ARRAY_SIZE; i++){
    //     array_put(&thread_data.sharedBuffer, test);
    // }
    // array_print(&thread_data.sharedBuffer);
    // printf("Buffer counter is %d\n" , array_position(&thread_data.sharedBuffer));
    // for(int i=0; i<ARRAY_SIZE; i++){
    //     array_get(&thread_data.sharedBuffer, &test);
    // }
    // array_print(&thread_data.sharedBuffer);

    printf("This test program produced %d items and consumed %d items \n",thread_data.sharedBuffer.num_produced, thread_data.sharedBuffer.num_consumed);
    pthread_mutex_destroy(&(thread_data.count_produced));
    pthread_mutex_destroy(&(thread_data.buffer_position));


    array_free(&thread_data.sharedBuffer);


    return 0;
}






