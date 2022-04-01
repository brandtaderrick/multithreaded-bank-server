/*
Derrick Brandt
Project 2
Bank Account Manager
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "Bank.h"

#define INFINITE 1
#define COMMANDSIZE 256
#define MAXLENGTH 512

// declare functions
void* workerThreadRequestHandler();
void freeStringArray(char** commandStringArray);
void parseUserRequest(char** commandString, int numInputs);
// void recordRequest();
// void presentInfoToUser();
// void processRequestInBackground();
// void printToFile();

// structure for a transaction pair
typedef struct Transaction {   
     //account id  &
     // amount to be added, could be positive or negative
     int acc_id;   
     int amount;  
     
} Transaction;

typedef struct Request{
    struct Request *S;  //pointer to the next request in the list
    int requestID;      //request ID assigned by the main thread
    int checkAccountID; //account ID for a check request
    Transaction* transactions; // array of transaction data
    int numberOfTransactions; //number of accounts in this transaction
    struct timeval startTime, endTime; //start time and end time for TIME
} Request;

typedef struct Queue{
    Request *head, *tail;
    int numberOfJobs; //number of jobs currently in queue
} Queue;

// What do I have to do here??
 Queue jobQueue;
 pthread_cond_t waitForReadyJob;
 pthread_mutex_t mutex;

int main(int argc, char**argv){
    
    // declare variables
    int numberOfWorkerThreads = 0;
    int numberOfAccounts = 0;
    FILE *filePointer;
    char* fileName;
    char* errorMessage1 = "Error: incorrect input. Exiting. \n";
    Request newRequest;
   

    //variables for user input
    char commandUserEnters[COMMANDSIZE];
    char** parsed_command = malloc(1); 
    //chunk/cut the string by spaces
    const char delimiter[2] = " "; 
    char* token;
    int i = 0;
    int stringLength;

    // conditioinally check for correct input
    if(argc != 4){
            printf("Number of arguments: %d\n", argc);
            printf("%s",errorMessage1);
            exit(1);
    }
    else{
        numberOfWorkerThreads = atoi(argv[1]);
        numberOfAccounts = atoi(argv[2]);
        fileName = argv[3];
    }

    printf("numthreads: %d\n", numberOfWorkerThreads);
    printf("numaccounts:%d\n", numberOfAccounts);
    printf("filename:%s\n", fileName);

    /*declare and initialize threads, mutex, conditions */
    pthread_t workerThreads[numberOfWorkerThreads];
    int threadIndex[numberOfWorkerThreads];
    //going to have to pass the accountMutex array to functions??
    pthread_mutex_t accountMutex[numberOfAccounts];
    pthread_cond_init(&waitForReadyJob, NULL);

    //create threads
    for(int i = 0; i < numberOfWorkerThreads; i++){
        threadIndex[i] = i;
        pthread_create(&workerThreads[i], NULL, workerThreadRequestHandler, (void *) &threadIndex[i]);
    }

    /*initialize one mutex per bank account */
    for(int j = 0; j < numberOfAccounts; j++){
         pthread_mutex_init(&accountMutex[j], NULL);
    }

    /* initialize actual bank accounts */
    if(initialize_accounts(numberOfAccounts) == 1){
        printf("All accounts successfuly initialized. \n");
    }

    /*initialize output file */
    filePointer = fopen(fileName, "w+");

    /*get user input*/
    while(INFINITE){
         // properly initialize each iteration
        parsed_command = malloc(1); 
        i = 0;
        // FGETS
        printf("%s", "Please enter your request: \n");
        fgets(commandUserEnters, COMMANDSIZE, stdin);  
        // get the length of the command to facilitate changing end character
        stringLength = strlen(commandUserEnters);
        commandUserEnters[stringLength - 1] = '\0';
        // get the first token
        token = strtok(commandUserEnters, delimiter);

        // walk through other tokens
        while(token != NULL){
                int size = i + 1;
                parsed_command = realloc(parsed_command, sizeof(char*) * size);
                parsed_command[i] = malloc(strlen(token) + 1);
                strcpy(parsed_command[i], token);
                i++;
                token = strtok(NULL, delimiter); //remembers last token
                
        }
        // need to allocate space for NULL in order to properly run execvp()
        parsed_command = realloc(parsed_command, sizeof(char*) * (i+1) );
        parsed_command[i] =  NULL;
        
        parseUserRequest(parsed_command, i);
        freeStringArray(parsed_command);

    }

}
void parseUserRequest(char** commandString, int numInputs){

    if(strcmp(commandString[0], "exit") == 0){
        printf("Exiting\n");
        exit(0);
    }
    else if(strcmp(commandString[0], "CHECK") == 0){
        pthread_cond_signal(&waitForReadyJob);
    }
    else if(strcmp(commandString[0], "TRANS") == 0){

    }
    else {
       printf("Unsupported input. Try again.\n");
    }

}


void* workerThreadRequestHandler(void *arg){

    pthread_mutex_lock(&mutex);
    // wait for the ready signal... should this come when we add a request to queue
    pthread_cond_wait(&waitForReadyJob, &mutex);

    int thread_ID = *((int*) arg);
    printf("Worker thread ID: %d\n", thread_ID);

    pthread_mutex_unlock(&mutex);

    /*Need to get jobs from queue here*/
    /*wait condition*/

    return NULL;
}

void freeStringArray(char** commandStringArray){
    int j = 0;

    while(commandStringArray[j] != NULL){
        free(commandStringArray[j]);
        j++;
    }
    free(commandStringArray);
}
