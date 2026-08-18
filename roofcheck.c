#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <wiringPi.h>
//#include <syslog.h>
#include <roofcheck.h>
//
#define REEDSWITCH 2
#define RELAY 25
//
int config(struct configparms * conf) ;
int email(struct configparms * conf) ;
int logmsg(char* message);
//int sms(struct configparms * conf) ;
//
int copy_file(const char *src_path, const char *dest_path);
int rooflog(int roofstate) ;
int ftp_send(struct configparms * conf) ;
int logpage(struct configparms * conf);
//
int setstatus(int roofstate) {
    FILE *dest = fopen("/var/gco/roofstatus.txt", "w");
    if (!dest) {
        return -2;
    }
    if (roofstate) {
        fwrite("OPEN\n", 1, 5, dest) ;
    } else {
        fwrite("CLOSED\n", 1, 7, dest) ;
    }
    fclose(dest);
    return 0; 
}   
    
//
// Volatile flag to ensure the compiler doesn't optimize it away in the loop
volatile sig_atomic_t keep_running = 1;
struct configparms conf , *conptr;

// Signal handler callback
void handle_signal(int signal) {
    switch (signal) {
        case SIGTERM:
            logmsg("received SIGTERM. Cleaning up and shutting down");
            keep_running = 0;
            break;
        case SIGHUP:
            logmsg("received SIGHUP. Reloading configuration files");
            conptr = &conf;
            config(conptr) ;
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[]) {
    int itest = 0;
    if( argc == 2 ) {
        if (!strcmp(argv[1],"test")) {
            itest = 1 ;
        }
    }
    struct sigaction act;
    // Set up the signal handler
    act.sa_handler = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    // Register handlers for both shutdown (SIGTERM) and reload (SIGHUP)
    if (sigaction(SIGTERM, &act, NULL) < 0) {
        perror("Error registering SIGTERM handler");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGHUP, &act, NULL) < 0) {
        perror("Error registering SIGHUP handler");
        return EXIT_FAILURE;
    }   //
    // get email configuration
    conptr = &conf;
    if (config(conptr)) {
        return 1  ;
    }    
    //
    char closedfile[50] ;
    char openfile[50] ;
    char statusfile[50];
    strcpy(closedfile,conf.szftpsource);
    strcpy(openfile,conf.szftpsource);
    strcpy(statusfile,conf.szftpsource);
    strcat(closedfile, "closed.html");
    strcat(openfile, "open.html");
    strcat(statusfile, "roofstatus.html");
    //
    if (itest) {
        conf.itest = 1 ;
    } 
    char message[200] ;
    sprintf(message,"----- Roof Monitor v2.1 - compiled on: %s %s -----", __DATE__, __TIME__);
    logmsg(message);
    //
    wiringPiSetup();
    pinMode(REEDSWITCH, INPUT);
    pullUpDnControl(REEDSWITCH, PUD_UP); // Enable the internal pull-up resistor
    pinMode(RELAY, OUTPUT);
    // exercise the relay
    digitalWrite(RELAY, 0);   // 0 = pull relay
    sleep(1);
    digitalWrite(RELAY, 1);   // 
    sleep(1);
    //
    //   Set relay to correct config on startup
    //
    FILE *roofstatus ;
    char status[50] ;    
    roofstatus = fopen( "/var/gco/roofstatus.txt", "r" ) ;
    if ( roofstatus == NULL ){
        logmsg( "Could not open file /var/gco/roofstatus.txt - exiting" ) ;
        return 1;
    }
    if( fgets ( status, 50, roofstatus ) != NULL ){
        if (itest) printf("Input from status file is %s \n", status) ;
        if (strncmp(status, "OPEN", 4) == 0)    //
            digitalWrite(RELAY, 0);  // 0 = relax active - roof open
        else
            digitalWrite(RELAY, 1);  // 1 = relax relay - roof closed
        fclose(roofstatus) ;   
    }
    //
    time_t heartbeat = 0;
    int openbounce = 0 ; //openbounce
    int closebounce = 0 ;
    char *bouncemsg = "reed switch bounce detected" ;
    //
    while ( keep_running == 1 ) {
        time_t rawtime;
        time (&rawtime);
        if (rawtime > heartbeat + 3599) {
            logmsg("----- hourly 'heartbeat' upload follows -----");
            ftp_send(conptr);
            heartbeat = rawtime ;
        }
        //****************************
        sleep(1);
        //
        // End of delay
        //****************************
        int ireedswitch  = 0;
        if (digitalRead(REEDSWITCH)){
            ireedswitch  = 1 ;   /// switch is open 
            if (closebounce) logmsg(bouncemsg) ;
            closebounce = 0 ;
        } else {
            if (openbounce) logmsg(bouncemsg) ;
            openbounce = 0 ;  // switch closed, reset openbounce count
        }
        if (itest) {
            sprintf(message,"input from reed switch is %d", ireedswitch) ;
            logmsg(message) ;
        }
        //
        roofstatus = fopen( "/var/gco/roofstatus.txt", "r" ) ;
        if ( roofstatus == NULL ){
            logmsg( "Could not open file /var/gco/roofstatus.txt - exiting" ) ;
            return 1;
        }
        if( fgets ( status, 50, roofstatus ) != NULL ){
            fclose(roofstatus) ;        
            if (itest) printf("Input from status file is %s \n", status) ;
            if ((strncmp(status, "CLOSED", 6) != 0) && (!ireedswitch)
                && (++closebounce > conf.debounce)){   // roof has just closed
                conf.roofstatus = ireedswitch ;
                if (itest) printf( "Roof has been closed\n" ) ;
                setstatus(ireedswitch) ;  // update the state file
                rooflog(ireedswitch) ; // update roof log ;
                logpage(conptr); // build html log page, roof.txt
                copy_file(closedfile, statusfile); // copy html page, inserting timestamp
                ftp_send(conptr); // send html pages to public server
                //gpioWrite(RELAY,0) ; // Change state of the LED
                //gpio_write(pi, RELAY,0) ;
                digitalWrite(RELAY, 1);  // 1 = relax relay
                if (conf.notify) email((conptr)); // send email
                logmsg("roof closed") ;
            
            } else {
                // roof needs to be open for 30 seconds continuously
                if ((strncmp(status, "OPEN", 4) != 0) && (ireedswitch)
                    && (++openbounce > conf.debounce)){ // roof has just opened
                    conf.roofstatus = ireedswitch ;
                    if (itest) printf( "Roof has been opened\n" ) ;
                    setstatus(ireedswitch) ;
                    rooflog(ireedswitch) ; // update roof log ;
                    logpage(conptr); // build html log page, roof.txt
                    copy_file(openfile,statusfile); // copy html page
                    ftp_send(conptr);
                    //gpioWrite(RELAY,1) ;
                    //gpio_write(pi, RELAY,1) ;
                    digitalWrite(RELAY, 0);   // 0 = pull relay
                    if (conf.notify) email(conptr);
                    logmsg("roof opened") ; 
                }
            }
        }
    }
    return 0;
}
