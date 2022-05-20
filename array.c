/* AUTHOR: Vignesh Chandrasehar */
#include "array.h"
#include "multi-lookup.h"

int array_init(stack *s){
    s->counter = 0; 
    s->operations = 0;
    s->num_produced = 0;
    s->num_consumed = 0;
    printf("Initializing Buffer...\n");

    for(int i=0; i<ARRAY_SIZE; i++){
        s->array[i] = (char *)malloc(MAX_NAME_LENGTH * sizeof(char));
    }

   
    //printf("Is this working?\n");
    pthread_mutex_init(&(s->mutex), 0);
    sem_init(&(s->empty), 0, ARRAY_SIZE);
    sem_init(&(s->full), 0, 0);
    return 0;
}   

//producer threads
int array_put(stack *s, char *hostname){

    //we should never exceed array size because producer should wait for a consumer to take out
    
    //if hostname is greater than max name length, return error
    if(strlen(hostname)>MAX_NAME_LENGTH){
        printf("Error: Cannot handle hostname\n");
        return -1;
    }

    sem_wait(&(s->empty)); //if empty == 0 then there are no empty slots to produce an item
    pthread_mutex_lock(&(s->mutex));
    /*enter crit section */
    //hostname is being produced by producer to fill buffer
    
    //printf("Counter is %d\n", s->counter);
    //printf("size of hostname: %lu\n", sizeof(hostname));
    if(strncpy(s->array[s->counter], hostname, MAX_NAME_LENGTH) == NULL){
        printf("string not found");
        return 0;
    }
    s->counter++;
    if(s->counter == ARRAY_SIZE){
        //printf("buffer is full\n");
    }
    s->num_produced++;
    s->operations++;
    //printf("\t thread %lu produced %s\n",pthread_self(), hostname);
    //printf("\t counter is: %d\n", s->counter);
    //printf("\t filled: %d / %d \n\n", s->counter,ARRAY_SIZE);
    //remainder section

    pthread_mutex_unlock(&(s->mutex));
    sem_post(&(s->full)); //added an element to buffer
    //printf("slot filled\n");
    return 0;

}  // place element into the array

//consumer threads
int array_get (stack *s, char **hostname){
    

    sem_wait(&(s->full)); //getting passed this
    fflush(stderr);
    pthread_mutex_lock(&(s->mutex));

    //exit 
    if(s->counter ==0){
        pthread_mutex_unlock(&(s->mutex));
        //sem_post(&(s->full));
        pthread_exit(0);
    }
    /*critical section */
    if( strncpy(*hostname,s->array[s->counter-1], MAX_NAME_LENGTH) ==NULL ){
        printf("Bad hostname, could not get\n");
        fflush(stderr);
        return -1;
    }
    
    //*hostname = s->array[s->counter-1]; //char*

    s->counter--; 
    if(s->counter == 0){ //producers should stop and consumers 
        //printf("buffer is empty\n");
    }
    s->operations++;

    // }
    //printf("\tthread  %lu consuming %s\n",pthread_self(), *hostname);
    s->num_consumed++;
    //printf("\tcounter is: %d\n", s->counter);
    //printf("\tfilled: %d / %d \n\n", s->counter,ARRAY_SIZE);

    /*remainder section */
    //free(temp);
    pthread_mutex_unlock(&(s->mutex));
    sem_post(&(s->empty));
    //free(temp);
    //printf("empty slot ava;ilable\n");
    //using same buffer so no need to free the space while we are producing and consuming

    return 0;
}  

//call after end of executions
void array_free(stack *s){
    printf("Operations: %d\n", s->operations);
    sem_destroy(&(s->empty));
    sem_destroy(&(s->full));
    pthread_mutex_destroy(&(s->mutex));
    for(int i=0; i<ARRAY_SIZE; i++){
        free(s->array[i]);
    }
    printf("produced %d items and consumed %d items \n",s->num_produced, s->num_consumed);
    fflush(stderr);
}

int array_position(stack *s){
    return s->counter;
}





//check: when running multiple threads: writing and reading concurrently
//Different scenarios, CPU, 
//consumer tried to access shared array (before producer)
//if array is empty, cannot consume
//if array is full, cannot produce
//producer condition: values produced
//consumer finish: if array is empty
//are there any producers running and array is empty then consumer can terminate