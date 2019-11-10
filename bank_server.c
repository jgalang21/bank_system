
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include "Bank.h"

#define MAX_ACC_ID 100

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

pthread_mutex_t mut; //lock for the queue, and then the accounts,
pthread_cond_t command; 	
pthread_cond_t worker;


void *workerThread(); 
int done = 0; 

struct queue queueList; //the queue for the overall program


int main(int argc, char **argv){
	int ts = 0;
	queueList.num_jobs = 0; //initially set it to 0
	queueList.head = NULL; //set head and tail to null
	queueList.tail = NULL;
	
	char input [200];
	 
	FILE *retFile;
	retFile = fopen(argv[3], "w"); //enable write to the file (3rd argument)
	
	//initialize worker threads
	pthread_t worker_tid[atoi(argv[2])];
	int thread_index[atoi(argv[2])]; //create a thread index for the worker
	
	int w = 0; 
	for(w = 0; w < atoi(argv[2]); w++){ //create the specified amount of workers
		thread_index[w] = w;
		pthread_create(&worker_tid[w], NULL, workerThread, &thread_index[w]);
	
	}
	

	if(argc != 4){
		 printf("Not enough arguments\n");
		 return(0);
	}
	
	//main should just put things in the queue, protect queue with mutexes, use a signal to whenever something gets added to the queue.
	//worker should have two states: 1. waiting 2. stopping when it knows no more jobs are coming
	
	int idCount = 1; 
	
	int sizeA, o = 0;
	
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
		
	
		if(strcmp(parts[0], "END") == 0){
		
		
		//queue the rest of the commands before returning
		
		return 0; 
		
		}
		
		
		else if(strcmp(parts[0], "CHECK") == 0 && atoi(parts[1]) < MAX_ACC_ID){
	
			printf("ID %d\n", idCount);
			
			struct timeval time; 
			gettimeofday(&time, NULL);
	
			struct job *toAdd;
			toAdd = (struct job*) malloc(sizeof(struct job)); 
			
			gettimeofday(&time, NULL);
			toAdd->time_arrival = time; //assign the arrival time
			
			//printf("time is: %ld.%06.ld\n", time.tv_sec, time.tv_usec);
			toAdd->request_ID = idCount; //provide it's request ID
			int toInt = atoi(parts[1]); //conver the provided string to an Integer for account lookup
			toAdd->check_acc_ID =read_account(toInt); //add the account ID to job
			
			
			pthread_mutex_lock(&mut);
			toAdd->next = NULL; 	
			
			if(queueList.num_jobs == 0){
				queueList.head = queueList.tail = toAdd;
			}
			else{
				queueList.tail->next = toAdd;
				queueList.tail = toAdd;
				
			}
			
			queueList.num_jobs++;
			
			pthread_mutex_unlock(&mut);
			//this might go in the worker thread
			//mutex unlock will go here later 
			
			pthread_cond_signal(&worker);
			
			
			
			flockfile(retFile); //lock the file
			usleep(2000); //makes sure nothing gets corrupted
			
			//print to output file the following: <idCount> BAL <balance>
			fprintf(retFile, "%d BAL %d\n", idCount, read_account(toInt));
			
			funlockfile(retFile); //unlock the file
			idCount++;
			
			//gettimeofday(&time, NULL);
			
			
							
		}
		
		else if (strcmp(parts[0], "TRANS") == 0 ){
	
			if((index-1) % 2 != 0){

				printf("Odd number of arguments, rejecting request\n");
			}

			else{
				printf("ID %d\n", idCount);
				
				/**
			struct job{
			int request_ID;  // request ID assigned by the main thread
			int check_acc_ID; // account ID for a CHECK request
			struct trans *transactions; // array of transaction data
			int num_trans; // number of accounts in this transaction
			struct timeval time_arrival, time_end; // arrival time and end time
			struct job *next; // pointer to the next request
				
				**/
			
				struct job *toAdd;
				toAdd = (struct job*) malloc(sizeof(struct job)); 
				//malloc toAdd and the transaction
				
				toAdd->request_ID = idCount;
				
				struct timeval time; 
				gettimeofday(&time, NULL);
				toAdd->time_arrival = time; //assign the arrival time
			
				int c, r = 0, check = 0, p = 0; 
				
				struct trans *temp = toAdd->transactions; 
				temp = (struct trans*) malloc(sizeof(struct trans)); 
				
				for(c = 2; c <= index; c += 2){
				
					int account = atoi(parts[c-1]); //get the first parameter
					int amnt = atoi(parts[c]); //get the second parameter
					
					if(account < MAX_ACC_ID && account > 0){ //makes sure the account is in range
						
						temp[r].acc_id = account; //assign the account id at the element
						temp[r].amount = amnt; //assign the amount at the element
						
						r++; //increase size of struct array
					
					/**
						//debugging
						printf("acc_id - %d\n", account);
						printf("amount - %d\n", amnt);
					**/
					
					}
				}
				
				
				toAdd->num_trans = r; //add the number of accounts in the transaction
				//toAdd->next = NULL; //have it point to NULL as the next one.
				
				
				if(queueList.num_jobs == 0){
					queueList.head = queueList.tail = toAdd;
				}
				else{
					queueList.tail->next = toAdd;
					queueList.tail = toAdd;
				
				}
			
				
				idCount++;
				
				queueList.num_jobs++;
				
				
				
				//submit the job here to the worker thread
				
				
				/**
					//debugging
					for(p = 0; p < r; p++){
				
						printf("%d\n", temp[p].acc_id);
						printf("%d\n", temp[p].amount);
										}
					printf("r: %d\n", r); 
					
					**/
		

				}	
			}
		
		else{
		
			printf("Invalid command\n");
		
		}
	
		
	}
	
	
	//free the malloc in the worker thread
	
	
	
	
	fclose(retFile);
	
	
}

//to make this run, join the thread
void *workerThread() {

	pthread_mutex_lock(&mut);
	
	printf("processing...\n");
	
	pthread_mutex_unlock(&mut);
		
}



	






