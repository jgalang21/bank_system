
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
	

	if(argc != 4){
		 printf("Not enough arguments\n");
		 return(0);
	}
	
	//main should just put things in the queue, protect queue with mutexes, use a signal to whenever something gets added to the queue.
	//worker should have two states: 1. waiting 2. stopping when it knows no more jobs are coming
	
	int idCount = 1; 
	
	initialize_accounts(atoi(argv[2]));
	
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
	
		int sizeA, o = 0;
		while(parts[o] != NULL){ //helps keep track of transactions properly
			sizeA++;
			o++;
		}
	
		if(strcmp(parts[0], "END") == 0){
		
		//queue the rest of the commands before returning
		
		return 0; 
		
		}
		
		/**
			int request_ID;  // request ID assigned by the main thread
	int check_acc_ID; // account ID for a CHECK request
	struct trans *transactions; // array of transaction data
	int num_trans; // number of accounts in this transaction
	struct timeval time_arrival, time_end; // arrival time and end time
	struct job *next; // pointer to the next request**/
		
	
		
		else if(strcmp(parts[0], "CHECK") == 0 && atoi(parts[1]) < MAX_ACC_ID){
	
			printf("ID %d\n", idCount);
			
			
			struct job toAdd; //initialize a job
			toAdd.request_ID = idCount; //provide it's request ID
			
			int toInt = atoi(parts[1]); //conver the provided string to an Integer for account lookup
			toAdd.check_acc_ID = read_account(toInt); //add the account ID to job
			toAdd.time_arrival = time; //FIX ------------------------------------------------------------
			toAdd.next = NULL; 
				
			
			if(queueList.num_jobs == 0){
				queueList.head = queueList.tail = &toAdd;
			}
			else{
				queueList.tail->next = &toAdd;
				queueList.tail = &toAdd;
				
			}
			
			queueList.num_jobs++;
			
			//mutex unlock will go here later
			
			flockfile(retFile); //lock the file
			usleep(2000); //makes sure nothing gets corrupted
			
			//print to output file the following: <idCount> BAL <balance>
			fprintf(retFile, "%d BAL %d\n", idCount, read_account(toInt));
			
			funlockfile(retFile); //unlock the file
			idCount++;
			//task.time_end = put endtime here 
							
		}
		
		else if (strcmp(parts[0], "TRANS") == 0 ){
	
			printf("ID %d\n", idCount);
			
			printf("parts length: %d\n", sizeA);
			
			
					
		/**
			int request_ID;  // request ID assigned by the main thread
	int check_acc_ID; // account ID for a CHECK request
	struct trans *transactions; // array of transaction data
	int num_trans; // number of accounts in this transaction
	struct timeval time_arrival, time_end; // arrival time and end time
	struct job *next; // pointer to the next request**/
			
			struct job toAdd; 
			toAdd.requestID = idCount;
			
			
			if(queueList.num_jobs == 0){
				queueList.head = queueList.tail = &toAdd;
			}
			else{
				queueList.tail->next = &toAdd;
				queueList.tail = &toAdd;
				
			}
			
			
			
			
			
			idCount++;
			
			queueList.num_jobs++;
			
		
		}
		
		else{
		
			printf("Invalid command\n");
		
		}
	
		
	}
	
	/**
			logic for TRANS
			struct trans tid; 
			
			int t;
			
			if(sizeA % 2 != 0){ //if we have an ODD number of transacations, one isn't going to be fulfilled so we can just ignore the last one
				sizeA--;
			}
			 
			for(t = 2 ; t < sizeA; t+=2){ //create the pairings
			
			tid.acc_id = atoi(parts[t-1]);
			tid.amount = atoi(parts[t]);
			
			printf("%d, %d\n", tid.acc_id, tid.amount);
			
			}
			
			sizeA = 0;
			**/
	
	
	fclose(retFile);
	
}



	






