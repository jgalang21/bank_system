

bank_server : Bank.c bank_server.c 
	cc -o bank_server  Bank.o bserver.o
	
Bank : Bank.c Bank.h
	cc -o Bank Bank.c
	
bserver : bank_server.c 
	cc -o bserver bank_server.c

clean : 
	rm -f *.o
