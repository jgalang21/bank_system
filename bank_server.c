
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


int main(int argc, char **argv){
	char input [200];
	 
	if(argc != 4){
	 printf("Not enough arguments\n");
	 return(0);
	}
	
	int r = initialize_accounts(atoi(argv[1]));
	
	printf("%d\n", r);
	
	
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
	
		
	}
	
	
	
}
