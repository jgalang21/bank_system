
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
	int type; //added this, 1 = TRANS, 0 = CHECK
};

struct queue {
	struct job *head, *tail;  // two pointers pointing to the head and tail of the queue.
	// add jobs after the tail, take jobs from the head
	int num_jobs; // number of jobs in current queue
};

/**
*
* the commands are essentially the producer functions
* the worker threads are essentially the consumers
* 
**/

pthread_mutex_t mut; //lock for the queue, and then the accounts,
pthread_mutex_t mut2; //lock for the queue itself
pthread_cond_t command; 	
pthread_cond_t worker;
FILE *retFile;
int idCount;

//pthread_mutex_t mutArr[];

void *workerThread(); 
int working = 0; 

struct queue queueList; //the queue for the overall program

int toInt;

pthread_mutex_t *mutexArr;


int main(int argc, char **argv){
	 
	int ts = 0;
	queueList.num_jobs = 0; //initially set it to 0
	queueList.head = NULL; //set head and tail to null
	queueList.tail = NULL;
	
	char input [200];
	 
	
	retFile = fopen(argv[3], "w"); //enable write to the file (3rd argument)
	
	
	//initialize worker threads
	pthread_t worker_tid[atoi(argv[1])];
	int thread_index[atoi(argv[1])]; //create a thread index for the worker
	
	int w = 0; 
	for(w = 0; w < atoi(argv[2]); w++){ //create the specified amount of workers
		thread_index[w] = w;
		pthread_create(&worker_tid[w], NULL, workerThread, (void *)&thread_index[w]);
	
	}
	

	
	if(argc != 4){
		 printf("Not enough arguments\n");
		 return(0);
	}
	
	//main should just put things in the queue, protect queue with mutexes, use a signal to whenever something gets added to the queue.
	//worker should have two states: 1. waiting 2. stopping when it knows no more jobs are coming
	
	idCount = 1; 
	
	int sizeA, o = 0;
	
	initialize_accounts(atoi(argv[2]));
	
	pthread_mutex_init(&mut, NULL);
	pthread_cond_init(&worker, NULL);

	
	sizeA = atoi(argv[2]);
	mutexArr = (pthread_mutex_t*)malloc(sizeA * sizeof(mutexArr)); //create array of jobs
	
	for(o = 0; o < sizeA; o++){
		pthread_mutex_init(&mutexArr[o], NULL);
	}
	
	printf("error1\n");
	while(1){
		printf("error2\n"); //segfault happens around here? ------------------------------------------------------------------
	
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
		
	
		if(strcmp(parts[0],  "END") == 0){
		
		int r = 0; 

/**
		for(r = 0; r < queueList.num_jobs; r++){
			 working = 1;
			pthread_cond_broadcast(&worker);

			pthread_join(worker_tid[r], NULL);
		}
		**/
		//queue the rest of the commands before returning
		sleep(1);
	
		return 0; 
		
		}
		
		
		else if(strcmp(parts[0], "CHECK") == 0 && atoi(parts[1]) < MAX_ACC_ID){	

			printf("< ID %d\n", idCount);
			
			struct timeval time; 
			gettimeofday(&time, NULL);
	
			struct job *toAdd;
			toAdd = (struct job*) malloc(sizeof(struct job)); 
			
			gettimeofday(&time, NULL);
			toAdd->time_arrival = time; //assign the arrival time
			
			//printf("time is: %ld.%06.ld\n", time.tv_sec, time.tv_usec);
			toAdd->request_ID = idCount; //provide it's request ID
			 toInt = atoi(parts[1]); //conver the provided string to an Integer for account lookup
			toAdd->check_acc_ID =read_account(toInt); //add the account ID to job
			toAdd->next = NULL; 	
			toAdd->type = 0;

			printf("%d\n", toAdd->type);
			
			if(queueList.num_jobs == 0){
				queueList.head = queueList.tail = toAdd;
			}
			else{
				queueList.tail->next = toAdd;
				queueList.tail = toAdd;
				
			}
			
			queueList.num_jobs++;
		
			idCount++;
			
			
							
		}
		
		else if (strcmp(parts[0], "TRANS") == 0 ){
	
			if((index-1) % 2  != 0){

				printf("Odd number of arguments, rejecting request\n");
			}

			else{
				printf("ID %d\n", idCount);
			
				struct job *toAdd;
				toAdd = (struct job*) malloc(sizeof(struct job)); 
				//malloc toAdd and the transaction
				
				toAdd->request_ID = idCount;
				
				struct timeval time; 
				gettimeofday(&time, NULL);
				toAdd->time_arrival = time; //assign the arrival time
			
				int c, r = 0, check = 0, p = 0; 
				
				struct trans *temp = toAdd->transactions; 
				temp = (struct trans*) malloc(sizeof(struct trans) * ((index-1))); 
				
				for(c = 2; c <= index-1; c += 2){
				
					int account = atoi(parts[c-1]); //get the first parameter
					int amnt = atoi(parts[c]); //get the second parameter
					
					
					if(account < MAX_ACC_ID && account > 0){ //makes sure the account is in range
						
						temp[r].acc_id = account; //assign the account id at the element
						temp[r].amount = amnt; //assign the amount at the element
						
						r++; //increase size of struct array
					
						//debugging
						//printf("acc_id - %d\n", account);
						//printf("amount - %d\n", amnt);
					
					
					}
				}
				
				toAdd->type = 1;
				toAdd->transactions = temp;
				toAdd->num_trans = r; //add the number of accounts in the transaction
				
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

				}	
			}
		
		else{
		
			printf("Invalid command\n");
		
		}
	
		pthread_cond_broadcast(&worker);
	}
	
	
	//free the malloc in the worker thread
	
	 
	fclose(retFile);
	
	
}

//to make this run, join the thread

void *workerThread(void *arg) {

while(queueList.num_jobs != 0){
	pthread_mutex_lock(&mut);
	
	printf("here i am\n");
	struct job* toCheck = queueList.head;

	if(toCheck->type == 0){ //if its a CHECK request
		printf("This is a CHECK request\n");

		flockfile(retFile); //lock the file
		usleep(2000); //makes sure nothing gets corrupted
		 	
		struct timeval time; 
		gettimeofday(&time, NULL);	
		toCheck->time_end = time;
		//print to output file the following: <idCount> BAL <balance>
		fprintf(retFile, "%d BAL %d %ld.%06.ld %ld.%06.ld \n", toCheck->request_ID, read_account(toInt), toCheck->time_arrival, toCheck->time_end);
			
		funlockfile(retFile); //unlock the file
		idCount++;

	}

	else if(toCheck->type == 1){ //if it's a TRANS request
		printf("This is a TRANS request\n");

		printf("getting here\n");
		printf("%d total\n", toCheck->num_trans);

		int isVoid = 0; //boolean that checks to make sure that all given accounts have sufficient balance.

		flockfile(retFile); //lock the file
		
		int cancel = 0; //check if we need to cancel the TRANS due to an insufficient balance
		
	
		int z = 0; 
		int fRID, tID, once = 0; 
		
		for(z = 0; z < toCheck->num_trans; z++){
			int tempAmt = toCheck->transactions[z].amount;
			int tempID = toCheck->transactions[z].acc_id;
		
			if(read_account(tempID) + tempAmt < 0  && once == 0){ //if the amount we want to add is negative
			 
			 cancel = 1;	
			 fRID = toCheck->request_ID;
			 tID = tempID;
			 once = 1; 
			
		
			}
			
		}
		
		if(once == 1){
			fprintf(retFile, "%d ISF %d\n", fRID, tID); //have it only print once
		}

		else if(cancel != 1){
			for(z = 0; z < toCheck->num_trans; z++){
				int tempAmt = toCheck->transactions[z].amount;
				int tempID = toCheck->transactions[z].acc_id;
		
				if(tempAmt > 0){ //if the amount we want to add is positive
	
					pthread_mutex_lock(&mutexArr[tempID-1]);
					fprintf(retFile, "%d OK\n", toCheck->request_ID); //have it only print once	
					pthread_mutex_unlock(&mutexArr[tempID-1]);
					write_account(tempID, tempAmt);
					//printf("acc_id: %d, balance: %d\n", tempID, read_account(tempID));		

				}
			
				else if(tempAmt < 0 && (read_account(tempID)+tempAmt >= 0)){ //if the amount we want to add is negative
					int newBalance = tempAmt +read_account(tempID) ;
			
					pthread_mutex_lock(&mutexArr[tempID-1]);
					fprintf(retFile, "%d OK\n", toCheck->request_ID); //have it only print once	
					pthread_mutex_unlock(&mutexArr[tempID-1]);
					write_account(tempID, newBalance);
					//printf("acc_id: %d, balance: %d\n", tempID, read_account(tempID));	
				
				}
			
			}
			
		}

	

		struct timeval time; 
		gettimeofday(&time, NULL);	
		toCheck->time_end = time;

			
		funlockfile(retFile); //unlock the file
		
	}


	//updating the queue here
	if(queueList.head->next != NULL){
		queueList.head = queueList.head->next;
	}

	if(queueList.tail->next != NULL){
		queueList.tail = queueList.tail->next;
	}

	//free(toCheck);

	fflush(stdin);
	queueList.num_jobs--;
	pthread_mutex_unlock(&mut);
	//return NULL;
		
		}
}



	






