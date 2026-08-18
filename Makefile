CC=gcc
CFLAGS=-c -Wall 
CURLLIBS=-lcurl
GPIOLIB=-lwiringPi
MATHLIB=-lm
all:roofcheck

roofcheck:roofcheck.o filecopy.o ftp.o log.o config.o email.o
	$(CC)  roofcheck.o  filecopy.o  ftp.o log.o config.o email.o -o roofcheck $(GPIOLIB) $(CURLLIBS)  $(MATHLIB) 

roofcheck.o:roofcheck.c
	$(CC) $(CFLAGS) roofcheck.c -o roofcheck.o

filecopy.o:filecopy.c
	$(CC) $(CFLAGS) filecopy.c -o filecopy.o
	

ftp.o:ftp.c
	$(CC) $(CFLAGS) ftp.c -o ftp.o	

log.o:log.c
	$(CC) $(CFLAGS) log.c -o log.o	

email.o:email.c
	$(CC) $(CFLAGS) email.c -o email.o		
	
config.o:config.c
	$(CC) $(CFLAGS) config.c -o config.o	
		
clean:
	rm -rf *o roofcheck
	
	
	
# gcc -Wall -pthread -o prog prog.c -lpigpiod_if2 -lrt
