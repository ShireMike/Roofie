#include<stdio.h>
#include<string.h>  //strlen
//#include <errno.h>
#include<stdlib.h>  //strlen
//#include<sys/socket.h>
//#include<arpa/inet.h>   //inet_addr
#include<unistd.h>  //write
//#include <signal.h>
#include<time.h>

int logmsg(char* message)
{
    char *logfile = "/var/log/roofcheck.log" ;
    FILE *roofcheck= NULL;
    char timestamp [30] ;
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(timestamp,30,"%Y-%m-%d %H:%M:%S",timeinfo);    
    roofcheck = fopen (logfile, "a");
    fprintf(roofcheck,"%s %s\n", timestamp, message) ;
    fclose(roofcheck);
    return 0 ;

}