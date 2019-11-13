
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

void *workerThread(); 
int working = 0; 

struct queue queueList; //the queue for the overall program
int toInt;

int main(int argc, char **argv){
	int ts = 0;
	queueList.num_jobs = 0; //initially set it to 0
	queueList.head = NULL; //set head and tail to null
	queueList.tail = NULL;
	
	char input [200];
	 
	
	retFile = fopen(argv[3], "w"); //enable write to the file (3rd argument)
	
	
	//initialize worker threads
	pthread_t worker_tid[atoi(argv[2])];
	int thread_index[atoi(argv[2])]; //create a thread index for the worker
	
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

	pthread_mutex_t mutexArr[atoi(argv[2])-1]; //create the array of mutexes
	
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
		
		int r = 0; 


		
		for(r = 0; r < queueList.num_jobs; r++){
			 working = 1;
			pthread_cond_broadcast(&worker);

			pthread_join(worker_tid[r], NULL);
		}
		//queue the rest of the commands before returning
	
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
			
			//this might go in the worker thread
			//mutex unlock will go here later 
			
			idCount++;
			
			
			//gettimeofday(&time, NULL);
			
			
							
		}
		
		else if (strcmp(parts[0], "TRANS") == 0 ){
	
			if((index-1) % 2 != 0){

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
				temp = (struct trans*) malloc(sizeof(struct trans)); 
				
				for(c = 2; c <= index; c += 2){
				
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
				printf("%d\n", toAdd->type);
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
	
		
	}
	
	
	//free the malloc in the worker thread
	
	
	fclose(retFile);
	
	
}

//to make this run, join the thread

void *workerThread(void *arg) {

	pthread_mutex_lock(&mut);
	while(working == 0)
		pthread_cond_wait(&worker, &mut);


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

		printf("%d total\n", toCheck->num_trans);

		struct trans *tempTrans = toCheck->transactions;


		flockfile(retFile); //lock the file

		int n = 0; 
		for(n = 0; n < toCheck->num_trans; n++){
		  if(tempTrans[n].amount > 0) {
		  	write_account(tempTrans[n].acc_id, tempTrans[n].amount);
			fprintf(retFile, "%d OK\n", toCheck->request_ID);
		  //	printf("positive vibes\n");
		  }
		  else if(tempTrans[n].amount < 0 && (read_account(tempTrans[n].acc_id)-tempTrans[n].amount > 0)){
		  	write_account(tempTrans[n].acc_id, tempTrans[n].amount);
		  	fprintf(retFile, "%d OK\n", toCheck->request_ID);

		 // 	printf("negative vibes :(\n");
		  }
		
		}
		struct timeval time; 
		gettimeofday(&time, NULL);	
		toCheck->time_end = time;
		//print to output file the following: <idCount> BAL <balance>
		//fprintf(retFile, "%d BAL %d %ld.%06.ld %ld.%06.ld \n", idCount);
			
		funlockfile(retFile); //unlock the file
		
	}


	//updating the queue here
	if(queueList.head->next != NULL){
		queueList.head = queueList.head->next;
	}

	if(queueList.tail->next != NULL){
		queueList.tail = queueList.tail->next;
	}



	fflush(stdin);

	pthread_mutex_unlock(&mut);
	return NULL;
		
}



	






