/* AUTHOR: Vignesh Chandrasehar */
//collaborator: Sidhant Puntambekar

#include "multi-lookup.h"
#include "array.h"
#define NOT_RESOLVED "NOT_RESOLVED"


/*producer reads from input files, puts on shared buffer using array_put
and writes to services.txt
*/
void* requesters(void* args){
    
    Requesters *data = (Requesters *) args;
    //printf("I am a requester\n");
    /* *************Local Vars************ */
    // unsigned long tid = pthread_self();
    pthread_t tid = pthread_self();
    int serviced_count = 0; //local count of the number of files serviced, independent for each requester
    int local_index = -1; //used by every thread

    while(1){

        /*************Critical Section****************/
        pthread_mutex_lock(&(data->file_position_lock)); 
        if( data->global_index>= data->file_count){ //if all files have been processed //data->file_count<=0
            pthread_mutex_unlock(&(data->file_position_lock));
            printf("Thread %lx serviced %d files\n", tid, serviced_count);
            fflush(stderr);
            return NULL;
        }
        pthread_mutex_unlock(&(data->file_position_lock)); //locking file index  

        /*************Critical Section************** */
        pthread_mutex_lock(&(data->file_position_lock)); 
        local_index = data->global_index;
        data->global_index++;
        pthread_mutex_unlock(&(data->file_position_lock));
        /***************************************************/
       
        char *filename;
        filename = data->input_files[local_index];
        //printf("thread %lu got file %s\n",tid, filename);
        FILE * fd = fopen(filename, "r+"); //only one thread should read into temp, other threadds should read other files
        //check if  file was successful
        if(fd == NULL){
            fflush(stderr);
            continue; //skip that file
        }
        char hostname[MAX_NAME_LENGTH];
        while(fgets(hostname, sizeof(hostname), fd)){
            //printf("%s\n", hostname);
            if( (hostname[0] != '\n')){
                hostname[strlen(hostname)-1]='\0';
                /*****************BOUNDED BUFFER************************/
                array_put(data->shared_buffer, hostname);
                /***************** Critical Section ********************/
                pthread_mutex_lock(&(data->serviced_lock));
                fprintf(data->serviced_log, "%s\n", hostname);
                fflush(stderr);
                pthread_mutex_unlock(&(data->serviced_lock));
            }
            
        }
        fclose(fd); //every thread will close its file after reading it
        serviced_count++; 
    }
    return NULL;
}



void *resolvers(void* args){

    /*local to the thread*/
    Resolvers *data = (Resolvers *) args;
    // unsigned long tid = pthread_self();
     pthread_t tid = pthread_self();
    char * name = malloc(MAX_NAME_LENGTH);
    //syscall error check
    if(name==NULL){
        printf("Could not malloc() hostname\n. Terminiating.....\n");
        exit(-1);
    }

    char * IP = malloc(MAX_IP_LENGTH);

    if(IP==NULL){
        printf("Could not malloc() IP address\n. Terminiating.....\n");
        exit(-1);
    }
    int resolved_count = 0;

    /*consume until I see a poison pill*/
    while(1){
        //terminate check here : VERSION POSION PILL
        if(array_get(data->shared_buffer, &name)!=0){ 
            pthread_mutex_lock(&(data->results_lock));
            fprintf(stderr, "Bad hostname\n");
            fflush(stderr);
            pthread_mutex_unlock(&(data->results_lock));
        }

        /* POSION PILL */
        printf("word: %s\n", name);
        if(strcmp(name, "PEACE OUT")==0){
            printf("Thread %lx resolved %d hostnames\n", tid, resolved_count);
            fflush(stderr);
            free(name);
            free(IP);
            return NULL;
        }

        if(dnslookup(name, IP, MAX_IP_LENGTH)==0){ //overhead alert
            /**************CRITICAL SECTION****************/
            pthread_mutex_lock(&(data->results_lock)); 
            fprintf(data->results_log, "%s, %s\n", name, IP);
            fflush(stderr);
            printf("FLUSH\n");
            pthread_mutex_unlock(&(data->results_lock));
            resolved_count++;
        }else{
            /**************CRITICAL SECTION****************/
            pthread_mutex_lock(&(data->results_lock)); 
            fprintf(data->results_log, "%s, %s\n", name, NOT_RESOLVED);
            fflush(stderr);
            pthread_mutex_unlock(&(data->results_lock));
        }
    }
    return NULL;
}



int main(int argc, char* argv[]){
    //part 1: file input and error handling
    struct timeval start, end;
    gettimeofday(&start, NULL);

    if(argc<6){
        printf("Error: Not enough arguments\n");
        return -1;
    }
    if(argc>MAX_INPUT_FILES +5){
        printf("Error: Too many arguments\n");
        return -1;
    }

    
    if(strlen(argv[1]) >2 || strlen(argv[2]) >2){
        printf("Error: User entered non integer requester or resolver\n");
        return -1;
    }
    int num_requesters = atoi(argv[1]);
    int num_resolvers = atoi(argv[2]);
    

    if(num_requesters<1 || num_resolvers <1 || num_requesters>MAX_REQUESTER_THREADS || num_resolvers>MAX_RESOLVER_THREADS){
        printf("Error: Invalid thread request\n");
        return -1;
    }

    printf("User wants %d requesters and %d resolvers\n", num_requesters, num_resolvers);



    //********************Processing names of input files*********************//
    //store the name of each input file in *input file, and pass as arg to requestsers

    char *file_list[argc-5]; //length of file array is amount in /input
    for(int i=0; i<argc-5; i++){
        //create a list of file names
        file_list[i] = malloc (sizeof(char) * MAX_NAME_LENGTH );
    }

    //skip invalid files and move to next one
    for(int i=5; i<argc; i++){
        if(access(argv[i], F_OK) !=0){
            printf("invalid file %s\n", argv[i]);
        }else{
            strcpy(file_list[i-5], argv[i]);

        }
        
    }

    //printing file list
    /*for(int i=0; i<argc-5; i++){
        printf("%s\n", file_list[i]);
    }*/

    stack buffer;
    array_init(&buffer);

    Requesters requester_args; //struct in .h
    Resolvers resolver_args; //struct in .h

    pthread_t requesters_id[num_requesters];
    pthread_t resolvers_id[num_resolvers];


//******************Initialize requester arguments**********************//////////////

    //testing pointer for file_list
    requester_args.input_files = file_list; //address of list of file names
    requester_args.file_count = argc-5; //when this hits argc-5 we requesters are done
    requester_args.global_index = 0;
    pthread_mutex_init((&requester_args.file_position_lock), 0); 
    pthread_mutex_init((&requester_args.serviced_lock), 0) ;


    requester_args.shared_buffer = &buffer; //data structure buffer defined as pointer
    requester_args.serviced_log = fopen(argv[3], "w+");
    int fdreq = access(argv[3], F_OK | W_OK);

    if(requester_args.serviced_log == NULL || fdreq!=0){
        printf("Error: Could not open requester log file\n");
        return -1;
    }

//**********************Initialize resolver arguments**********************//////////////////
    
    pthread_mutex_init((&resolver_args.results_lock), 0); 
    resolver_args.shared_buffer = &buffer;
    resolver_args.results_log = fopen(argv[4], "w+");
    int fdres = access(argv[4], F_OK | W_OK);

    if(resolver_args.results_log == NULL || fdres !=0){
        printf("Error: Could not open resolver log file\n");
        return -1;
    }

    

//**********************Create Threads*****************************///
    for(int i=0; i<num_requesters; i++){
        if(pthread_create(&requesters_id[i], NULL, requesters, &requester_args) !=0){
            printf("Failed to create requester thread\n");
        }

    }

    for(int i=0; i<num_resolvers; i++){
        if(pthread_create(&resolvers_id[i], NULL, resolvers, &resolver_args)!=0){
            printf("Failed to create resolver thread\n");
        }
    }
    

    for(int i=0; i<num_requesters; i++){
        if(pthread_join(requesters_id[i], NULL)!=0){
            printf("Failed to join requester\n");
        }
    }
    /*printf("Stack position: %d\n", buffer.counter);
    for(int i=0; i<buffer.counter; i++){
        printf("%s, ", buffer.array[i]);
        
    }
    printf("\n"); */

    /* THE HOLY GRAIL => BYE RESOLVERS */
    //wait until buffer is empty before injecting poison
    while (buffer.counter != 0);
    for(int i=0; i<num_resolvers; i++){
        array_put(&buffer, "PEACE OUT"); /*POSION PILL*/
    }

    /*for(int i=0; i<buffer.counter; i++){
        printf("%s, ", buffer.array[i]);
        
    }
    printf("\n"); */

    for(int i=0; i<num_resolvers; i++){
        if(pthread_join(resolvers_id[i], NULL)!=0){
            printf("Failed to join resolver\n");
        }
    }
    

    pthread_mutex_destroy(&(requester_args.file_position_lock));
    pthread_mutex_destroy(&(requester_args.serviced_lock));
    pthread_mutex_destroy(&(resolver_args.results_lock));


   
    for(int i=0; i<argc-5; i++){
        free(file_list[i]);
    }
    array_free(&buffer);

    fclose(requester_args.serviced_log);
    fclose(resolver_args.results_log);
    

    gettimeofday(&end, NULL);
    float time = (end.tv_sec - start.tv_sec) + 1e-6*(end.tv_usec - start.tv_usec);
    printf("./multi-lookup: total time is %f seconds\n", time);
    //printf("Stack position: %d\n", buffer.counter);

    return 0;
}
