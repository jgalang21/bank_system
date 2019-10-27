

bank_server : Bank.o bank_server.o 
	cc -pthread -o bank_server Bank.o bank_server.o
	
Bank : Bank.c 
	cc -c Bank.c 
	
bserver : bank_server.c 
	cc -c bank_server.c

clean : 
	rm -f *.o
