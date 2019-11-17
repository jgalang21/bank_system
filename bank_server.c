#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include "Bank.h"



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

pthread_mutex_t mut; //lock for the queue, and then the accounts,
pthread_mutex_t mut2; //lock for the queue itself

pthread_cond_t worker;
FILE *retFile;
int idCount;
void *workerThread(); 
int working = 0; 

struct queue queueList; //the queue for the overall program

int toInt;
int maxAccounts; //max accounts for error checking
pthread_mutex_t *mutexArr; //array of mutex locks for each account


int main(int argc, char **argv){
	 
	 if(argc != 4){ //error checking
		 printf("Not enough arguments\n");
		 return(0);
	}
	
	int ts = 0;
	queueList.num_jobs = 0; //initially set it to 0
	queueList.head = NULL; //set head and tail to null
	queueList.tail = NULL;
	
	char input [200];

	retFile = fopen(argv[3], "w"); //enable write to the file (3rd argument)
	
	pthread_t worker_tid[atoi(argv[1])]; //worker_tid array
	
	int thread_index[atoi(argv[1])]; //create a thread index for the worker
	int arg2 = atoi(argv[1]);
	

	int w = 0; 
	for(w = 0; w < arg2; w++){ //create the specified amount of workers
		pthread_create(&worker_tid[w], NULL, workerThread, (void *) &thread_index[w]);	
		thread_index[w] = w;	
	
	}
	
	
	idCount = 1; //keeps track of each individual unique ID per job assignment
	
	initialize_accounts(atoi(argv[2])); //initialize the accounts
	 maxAccounts = atoi(argv[2]);
	int sizeA, g = 0; //for creating the mutex array
	
	sizeA = atoi(argv[2]); //mutex array size
	mutexArr = (pthread_mutex_t*)malloc(sizeA * sizeof(mutexArr)); 
	
	for(g = 0; g < sizeA; g++){ //creating the mutex array
		pthread_mutex_init(&mutexArr[g], NULL);
	}
	
	pthread_mutex_init(&mut, NULL);
	pthread_cond_init(&worker, NULL);

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
		
		if(strcmp(parts[0],  "END") == 0){
		
		//finish the rest of the jobs, didn't figure out how to implement this sadly
		
		return 0; 
		
		}
	
		//making sure the account we're checking is within bounds
		else if(strcmp(parts[0], "CHECK") == 0 && (atoi(parts[1]) > 0 && atoi(parts[1]) <= maxAccounts)){	

			printf("< ID %d\n", idCount);
			
			struct timeval time; //timestamp
			gettimeofday(&time, NULL);
	
			struct job *toAdd; //allocating memory
			toAdd = (struct job*) malloc(sizeof(struct job)); 
			
			gettimeofday(&time, NULL);
			toAdd->time_arrival = time; //assign the arrival time

			toAdd->request_ID = idCount; //provide it's request ID
			toInt = atoi(parts[1]); //conver the provided string to an Integer for account lookup
			toAdd->check_acc_ID =read_account(toInt); //add the account ID to job
			toAdd->next = NULL; //the next job should be null
			toAdd->type = 0;

			pthread_mutex_lock(&mut); //lock the queue
			if(queueList.num_jobs == 0){ //update the queue
				queueList.head = queueList.tail = toAdd;
			}
			else{
				queueList.tail->next = toAdd;
				queueList.tail = toAdd;
				
			}
			pthread_mutex_unlock(&mut); //unlock the queue
			
			working = 1; 
			queueList.num_jobs++;
			pthread_cond_broadcast(&worker); //broadcast to the worker thread	
	
			idCount++;
			workerThread(); 
				
							
		}
		
		else if (strcmp(parts[0], "TRANS") == 0 ){
	
			if((index-1) % 2 != 0){ //error checking

				printf("Odd number of arguments, rejecting request\n");
			}

			else{
				printf("ID %d\n", idCount);
			
				struct job *toAdd;
				toAdd = (struct job*) malloc(sizeof(struct job)); 
				//malloc toAdd and the transaction
				
				toAdd->request_ID = idCount; //assign the request ID
				
				struct timeval time;  
				gettimeofday(&time, NULL);
				toAdd->time_arrival = time; //assign the arrival time
			
				int c, r = 0, check = 0, p = 0; 
				
				struct trans *temp = toAdd->transactions; //allocate memory
				temp = (struct trans*) malloc(sizeof(struct trans) * ((index-1))); 
				
				for(c = 2; c <= index-1; c += 2){
				
					int account = atoi(parts[c-1]); //get the first parameter
					int amnt = atoi(parts[c]); //get the second parameter
					
					if(account <= maxAccounts && account > 0){ //makes sure the account is in range
						
						temp[r].acc_id = account; //assign the account id at the element
						temp[r].amount = amnt; //assign the amount at the element
						
						r++; //increase size of struct array
					}
				}
				
				toAdd->type = 1;
				toAdd->transactions = temp;
				toAdd->num_trans = r; //add the number of accounts in the transaction
				
				
				pthread_mutex_lock(&mut); //lock the queue
				if(queueList.num_jobs == 0){
					queueList.head = queueList.tail = toAdd;
				}
				else{
					queueList.tail->next = toAdd;
					queueList.tail = toAdd;
				
				}
				pthread_mutex_unlock(&mut);
				
				idCount++;
			
				queueList.num_jobs++;
				working = 1; 
				//pthread_cond_broadcast(&worker); for some reason commenting this out fixes my issues on mac, not sure about lab PCs

				workerThread();
				free(temp); 
				}	

			}
		
		else{
		
			printf("Invalid command or account doesn't exist.\n");
		
		}

	}
	 
	fclose(retFile);
	
}



void *workerThread(void *arg) {

while(working == 0){ //wait to receive a job
	pthread_cond_wait(&worker, &mut);

}

	pthread_mutex_lock(&mut); 
	struct job* toCheck = queueList.head; 
	
	if(toCheck->type == 0){ //if its a CHECK request

		flockfile(retFile); //lock the file
		
		struct timeval time;  //timestamp
		gettimeofday(&time, NULL);	
		toCheck->time_end = time;
		//print balance
		fprintf(retFile, "%d BAL %d %ld.%06.ld %ld.%06.ld \n", toCheck->request_ID, read_account(toInt), toCheck->time_arrival, toCheck->time_end);
	
		funlockfile(retFile); //unlock the file	

	}

	else if(toCheck->type == 1){ //if it's a TRANS request

		int isVoid = 0; //boolean that checks to make sure that all given accounts have sufficient balance.

		flockfile(retFile); //lock the file
		
		int cancel = 0; //check if we need to cancel the TRANS due to an insufficient balance
		
		int z = 0; 
		int fRID, tID, once = 0; 
		
		for(z = 0; z < toCheck->num_trans; z++){ //this for loop verifies that every transaction will be valid, otherwise we're rejecting the entire request
			int tempAmt = toCheck->transactions[z].amount;
			int tempID = toCheck->transactions[z].acc_id;
		
			if(read_account(tempID) + tempAmt < 0){ //if the amount we want to add is negative
			 
			 cancel = 1;	
			 fRID = toCheck->request_ID;
			 tID = tempID;
			 once = 1; 

			}
			
		}
		
		if(once == 1){ //reject the request
			struct timeval time; 
			gettimeofday(&time, NULL);	//timestamp the request
			toCheck->time_end = time;
			fprintf(retFile, "%d ISF %d %ld.%06.ld %ld.%06.ld\n", fRID, tID, toCheck->time_arrival, toCheck->time_end); //have it only print once
			
		}

		else if(cancel != 1 ){ //accept the request as long as the accounts
			int tempAmt; 
			int tempID;
			
			for(z = 0; z < toCheck->num_trans; z++){
				int tempAmt = toCheck->transactions[z].amount;
				int tempID = toCheck->transactions[z].acc_id;
		
				if(tempAmt > 0){ //if the amount we want to add is positive
					write_account(tempID, tempAmt);	

				}
			
				else if(tempAmt < 0 && (read_account(tempID)+tempAmt >= 0)){ //if the amount we want to add is negative
					int newBalance = tempAmt +read_account(tempID) ;
					write_account(tempID, newBalance);
		
				}
			
			}
			struct timeval time; 
			gettimeofday(&time, NULL);	//timestamp the end time
			toCheck->time_end = time;
			
			pthread_mutex_lock(&mutexArr[tempID-1]);
			fprintf(retFile, "%d OK %ld.%06.ld %ld.%06.ld\n", toCheck->request_ID, toCheck->time_arrival, toCheck->time_end); //have it only print once
			pthread_mutex_unlock(&mutexArr[tempID-1]);
			
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

	free(toCheck);
	
	working = 0;
	fflush(stdin);
	queueList.num_jobs--; //decrement the number of jobs in the queue
	
	pthread_mutex_unlock(&mut);
	

}

