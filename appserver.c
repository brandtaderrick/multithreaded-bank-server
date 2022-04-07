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
#include <sys/time.h>
#include "Bank.h"

#define INFINITE 1
#define COMMANDSIZE 256
#define MAXLENGTH 512

// declare functions
void* workerThreadRequestHandler();
void freeStringArray(char** commandStringArray);
void parseUserRequest(char** commandString, int numInputs);
void push();

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
    struct Request *nextRequest;  //pointer to the next request in the list
    int requestID;      //request ID assigned by the main thread
    int checkAccountID; //account ID for a check request
    Transaction* transactions; // array of transaction data
    int numberOfTransactions; //number of accounts in this transaction
    struct timeval startTime, endTime; //start time and end time for TIME
} Request;

Request* pop();

typedef struct Queue{
    Request *head, *tail;
    int numberOfJobs; //number of jobs currently in queue
} Queue;

// What do I have to do here??
 Queue jobQueue = {.head = NULL, .tail = NULL, .numberOfJobs = 0};
 pthread_cond_t forJobReady;
 pthread_mutex_t* accountLocks;
 pthread_mutex_t queueMutex;
 int requestCounter = 1;
 FILE *filePointer;
 char* fileName;

// START
int main(int argc, char**argv){
    
    // declare variables
    int numberOfWorkerThreads = 0;
    int numberOfAccounts = 0;
    char* errorMessage1 = "Error: incorrect input. Exiting. \n";
   

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
    pthread_cond_init(&forJobReady, NULL);
    pthread_mutex_init(&queueMutex, NULL);

    //create threads
    for(int i = 0; i < numberOfWorkerThreads; i++){
        threadIndex[i] = i;
        pthread_create(&workerThreads[i], NULL, workerThreadRequestHandler, (void *) &threadIndex[i]);
    }

    /*initialize one mutex per bank account */
    accountLocks = malloc(sizeof(pthread_mutex_t) * numberOfAccounts);

    for(int j = 0; j < numberOfAccounts; j++){
         pthread_mutex_init(&accountMutex[j], NULL);
    }

    /* initialize actual bank accounts */
    if(initialize_accounts(numberOfAccounts) == 1){
        printf("All accounts successfuly initialized. \n");
    }

    /*initialize output file */
    filePointer = fopen(fileName, "w+");
    fclose(filePointer);

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

    // END
}
void parseUserRequest(char** commandString, int numInputs){

    if(strcmp(commandString[0], "end") == 0){
        printf("Exiting\n");
        exit(0);
    }
    else if(strcmp(commandString[0], "CHECK") == 0){
         Request *req = malloc(sizeof(Request));
         struct timeval time;
         gettimeofday(&time, NULL);
         req->startTime = time;
         req->requestID = requestCounter;
         req->checkAccountID = atoi(commandString[1]);
         req->numberOfTransactions = 0;
        //  end time still needs to be completed in worker thread
        // next request still needs to be completed in push thread
        printf("MAIN--ID %d\nThe value of the request is: %p\n",requestCounter, req);
        requestCounter++;
        push(req);
        sleep(1);
        // pthread_cond_signal(&waitForReadyJob);
    }
    else if(strcmp(commandString[0], "TRANS") == 0){
        Request *req = malloc(sizeof(Request));
        Transaction* trans = malloc(sizeof(Transaction) *(numInputs));
        struct timeval time;
        gettimeofday(&time, NULL);
        req->startTime = time;
        req->requestID = requestCounter;
        // trans->acc_id = atoi(commandString[1]); 
        // trans->amount = atoi(commandString[2]);
        

      

        int i = 1;
        int counter = 0;
        while(commandString[i] != NULL){
            if(commandString[i] == NULL){
                printf("uh oh NULL");
            }
            printf("uh oh stinkyyyy");
            trans[counter].acc_id = atoi(commandString[i]);
            trans[counter].amount = atoi(commandString[i + 1]);
            printf("check command string parse: %d", atoi(commandString[i]));
            printf("check command string parse: %d", atoi(commandString[i + 1]));
            req->numberOfTransactions++;
            req->transactions = trans;
            i = i + 2;
            counter++;
        }

        
        printf("Account ID: %d\n Amount to be added: %d\n",req->transactions[0].acc_id, req->transactions[0].amount);
        printf("Account ID: %d\n Amount to be added: %d\n",req->transactions[1].acc_id, req->transactions[1].amount);

        printf("number of account in transaction equals? %d\n", req->numberOfTransactions);
        requestCounter++;
        push(req);
        sleep(1);
    }
    else {
       printf("Unsupported input. Try again.\n");
    }

}


void* workerThreadRequestHandler(void *arg){

    
    // wait for the ready signal... should this come when we add a request to queue
    // pthread_cond_wait(&waitForReadyJob, &mutex);

    int thread_ID = *((int*) arg);
    printf("Worker thread ID: %d\n", thread_ID);

    // pthread_mutex_unlock(&accountLocks[account_ID]);
for(;;){
    pthread_mutex_lock(&queueMutex); 
    
        while(jobQueue.numberOfJobs == 0){
            pthread_cond_wait(&forJobReady, &queueMutex);
        }
        
            Request *request = pop();

            //if number of transactions == 0 then this is a CHECK request
            if(request->numberOfTransactions == 0){
                printf("POP--Worker number %d handling the job! The number of jobs in the queue: %d\n", thread_ID ,jobQueue.numberOfJobs);
                // pthread_mutex_lock(&accountLocks[request->checkAccountID]);
                //testing
                printf("The value of the request is: %p\n", request);
                printf("Request ID: %d\n\n", request->requestID);

                struct timeval time;
                gettimeofday(&time, NULL);
                request->endTime = time;
                filePointer = fopen(fileName, "a");
                // requestID, account balance, start time, end time
                fprintf(filePointer, "%d BAL %d %ld.%06.ld %ld.%06.ld\n",request->requestID, read_account(request->checkAccountID), request->startTime, request->endTime);
                fclose(filePointer);
                fflush(stdout);
            }
            else if(request->numberOfTransactions > 0){
                printf("Worker thread reached transaction code\n");

                for(int i = 0; i < request->numberOfTransactions; i++){

                        // get account information and check if balanace can handle a subtraction
                    int account_balance = read_account(request->transactions[i].acc_id);
                    printf("account balance correctly received with new syntax??? : %d", account_balance);
                    if((account_balance + request->transactions[i].amount) < 0){
                        printf("Denied. Insufficient Funds");
                        struct timeval time;
                        gettimeofday(&time, NULL);
                        request->endTime = time;
                        filePointer = fopen(fileName, "a");
                        // requestID, account balance, start time, end time
                        //             RequestID  ISF <accountID> <start time> <end time>
                        fprintf(filePointer, "%d ISF %d %ld.%06.ld %ld.%06.ld\n",request->requestID, request->transactions[i].acc_id, request->startTime, request->endTime);
                        fclose(filePointer);
                        fflush(stdout);

                    }
                    else
                    {
                        // the transaction number is either positive or negative, so always add
                        write_account(request->transactions[i].acc_id, (account_balance + request->transactions[i].amount));
                        // successful transaction: write corresponding successful transaction string to log file
                        struct timeval time;
                        gettimeofday(&time, NULL);
                        request->endTime = time;
                        filePointer = fopen(fileName, "a");
                        // requestID, account balance, start time, end time
                        fprintf(filePointer, "%d OK TIME %ld.%06.ld %ld.%06.ld\n",request->requestID, request->startTime, request->endTime);
                        fclose(filePointer);
                        fflush(stdout);
                        printf("Read account: %d\n", read_account(request->transactions[i].acc_id));
                    }

                }

                

            }
            else{
                printf("ERROR. ABORT.");
                exit(1);
            }
        // pthread_mutex_unlock(&accountLocks[account_ID]);
        // pthread_mutex_unlock(&accountLocks[request->checkAccountID]);
        pthread_mutex_unlock(&queueMutex);

        // pthread_cond_destroy(&forJobReady);
        // pthread_cond_init(&forJobReady, NULL);
        

        /*Need to get jobs from queue here*/
        /*wait condition*/
        // print to file
        }
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

Request* pop(){

  if(jobQueue.numberOfJobs == 0){
      return NULL;
  }
  else if(jobQueue.numberOfJobs == 1){
      Request* result = jobQueue.head;
      jobQueue.head = NULL;
      jobQueue.tail = NULL;
      jobQueue.numberOfJobs--;
    //   returns Request struct
      return result;

  }
  else{
      Request *result = result = jobQueue.head;
      jobQueue.head = jobQueue.head->nextRequest;
      jobQueue.numberOfJobs--;
      return result;
  }
}

void push(Request *newRequest){
    //testing
    printf("PUSH--I am pushing to the queue\nThe requestID = %d\nThe check accountID = %d\nStart time = %ld.%06.ld\n",newRequest->requestID, newRequest->checkAccountID, newRequest->startTime.tv_sec, newRequest->startTime.tv_usec);
    

      if(jobQueue.numberOfJobs == 0){
        jobQueue.head = newRequest;
        jobQueue.tail = newRequest;
        printf("The value of the head is: %p\n", jobQueue.head);
        printf("The value of the tail is: %p\n", jobQueue.tail);
        jobQueue.numberOfJobs++;
    }
    else if(jobQueue.numberOfJobs > 0){
        jobQueue.tail->nextRequest = newRequest;
        jobQueue.tail = newRequest;
        jobQueue.numberOfJobs++;
    }

    //testing
    printf("Number of jobs in the queue: %d\n", jobQueue.numberOfJobs);
    printf("The value of the request is: %p\n", newRequest);
    pthread_cond_broadcast(&forJobReady);

    
}