
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include "Bank.h"

#define MAX_ACC_ID 99999

struct trans{    // structure for a transaction pair
	int acc_id;  //    account id
	int amount;  //    amount to be added, can be positive or negative
};

struct job{
	int request_ID;  // request ID assigned by the main thread
	int check_acc_ID; // account ID for a CHECK request
	struct trans *transactions; // array of transaction data
	int num_trans; // number of accounts in this transaction
	struct timeval time_arrival, time_end; // arrival time and end time
	struct job *next; // pointer to the next request
};

struct queue {
	struct job *head, *tail;  // two pointers pointing to the head and tail of the queue.
	// add jobs after the tail, take jobs from the head
	int num_jobs; // number of jobs in current queue
};


	/**
	int request_ID;  // request ID assigned by the main thread
	int check_acc_ID; // account ID for a CHECK request
	struct trans *transactions; // array of transaction data
	int num_trans; // number of accounts in this transaction
	struct timeval time_arrival, time_end; // arrival time and end time
	struct job *next; // pointer to the next request**/
	


/**
*
* the commands are essentially the producer functions
* the worker threads are essentially the consumers
* 
**/

pthread_mutex_t mut;
pthread_cond_t command; 	
pthread_cond_t worker;

//struct timeval time; 
struct queue queueList; //the queue for the overall program
struct timeval time; 


int main(int argc, char **argv){
	int ts = 0;
	queueList.num_jobs = 0; //initially set it to 0
	queueList.head = NULL; //set head and tail to null
	queueList.tail = NULL;
	
	char input [200];
	 
	FILE *retFile;
	retFile = fopen(argv[3], "w"); //enable write to the file (3rd argument)
	
	/**
	pthread_t work_tid[atoi(argv[1])]; 
	pthread_t com_tid;
	
	
	pthread_cond_t work_cv[atoi(argv[1])]; //worker conditional variable
	pthread_cond_t com_cv;  //command conditional variable
	int thread_index[TOTAL_CONSUMER_THREADS];
	**/
	
	if(argc != 4){
		 printf("Not enough arguments\n");
		 return(0);
	}
	
	int r = initialize_accounts(atoi(argv[1])); //initialize the accounts
	
	/**
	pthread_mutex_init(&mut, NULL);
	pthread_cond_init(&worker, NULL);
	pthread_cond_init(&command, NULL);
	
	pthread_create(&com_cv, NULL, command, NULL); //create command thread
	int k = 0; 
	
	for(k = 0; k < atoi(argv[1]); k++){
	pthread_create(&work_tid[k], NULL, worker, (void *) &thread_index[k]); 
	
	}
	**/
	
	
	//main should just put things in the queue, protect queue with mutexes, use a signal to whenever something gets added to the queue.
	//worker should have two states: 1. waiting 2. stopping when it knows no more jobs are coming
	
	
	
	printf("%d\n", r);
	
	int idCount = 1; 
	
	while(1){

		printf(">");
		
		fgets(input, 200, stdin); //stdin takes user input until user presses enter
		int len = strlen(input);
		input[len-1] = '\0'; //assign the last element to be null
		char* parts[len]; 
		int index = 0; 
		char* part = strtok(input, " "); //split the string
	
		while(part != NULL){ //given on the board on how to split a string
			parts[index] = part;
			index++;
			part = strtok(NULL, " ");
			parts[index] = NULL;
		}
	
	
		if(strcmp(parts[0], "END") == 0){
		
		//queue the rest of the commands before returning
		
		int k = 0; 
		
		for(k = 0; k < queueList.num_jobs; k++){
			printf("queue: %d\n", queueList.tail[k].request_ID);
		}
		
		
		return 0; 
		
		}
		
	
		
		else if(strcmp(parts[0], "CHECK") == 0 && atoi(parts[1]) < MAX_ACC_ID){
		/**
			struct job{
	int request_ID;  // request ID assigned by the main thread
	int check_acc_ID; // account ID for a CHECK request
	struct trans *transactions; // array of transaction data
	int num_trans; // number of accounts in this transaction
	struct timeval time_arrival, time_end; // arrival time and end time
	struct job *next; // pointer to the next request
};**/
		
			printf("ID %d\n", idCount);
			
			
			struct job toAdd; //initialize a job
			toAdd.request_ID = idCount; //provide it's request ID
			toAdd.check_acc_ID = read_account(atoi(parts[1]));
			toAdd.time_arrival = time;	
			toAdd.next = NULL;
				
			
			//mutex lock will go here later	
			if(queueList.tail == NULL){ //if the queue is empty
				queueList.tail = &toAdd;	
				printf("Next was null, so it's now the tail\n");
			}	
			
			else {
				struct job* current;
				current = queueList.tail;
				while(current != NULL){
				 current = queueList.tail[ts].next;
				}
				printf("Got here\n");
				
				
			
			}
				
			
			queueList.num_jobs++; //increase queue count jobs
			
			printf("Queue size: %d\n", queueList.num_jobs);
			printf("CheckAccID: %d\n", toAdd.check_acc_ID);
			printf("RequestID: %d\n", toAdd.request_ID);
			
			idCount++;
			
			//mutex unlock will go here later
			
			flockfile(retFile); //lock the file
			
			//print to output file the following: <idCount> BAL <balance>
			fprintf(retFile, "%d BAL %d\n", idCount, read_account(atoi(parts[1])));
			
			funlockfile(retFile); //unlock the file

			//task.time_end = put endtime here 
							
		}
		
		else if (strcmp(parts[0], "TRANS") == 0 ){
		
			printf("ID %d\n", idCount);
			idCount++;
			
			queueList.num_jobs++;
			
		
		}
		
		else{
		
			printf("Invalid command\n");
		
		}
	
		
	}
	
	
	
	
	fclose(retFile);
	
}



	






