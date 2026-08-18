#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roofcheck.h>
#include <errno.h>
#include <unistd.h> 
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>  
//
int logmsg(char* message);
//
char* generate_message_id(const char* domain)
{
    static char msg_id[300];
    struct timeval tv;
    pid_t pid = getpid();
    gettimeofday(&tv, NULL);
    unsigned long long ts = (unsigned long long)tv.tv_sec * 1000000ULL + tv.tv_usec;
    snprintf(msg_id, sizeof(msg_id), 
             "<%llu.%d.%d@%s>",
             ts,
             (int)pid,
             rand() % 100000,
             domain ? domain : "localhost");
    return msg_id;
}
//
int istimestamp(char *n) {

    int i = strlen(n)  -1 ;
    if (i != 10) return 0 ;
    int isnum = (i>0);
    while (i-- && isnum) {
        if (!(n[i] >= '0' && n[i] <= '9')) {
            isnum = 0;
        }
    }
    return isnum;
}
//
int email(struct configparms * conf)
{
    char message[200] ;   //   log message
    time_t rawtime;
    
    struct tm * timeinfo;
    char datetime [80];
    char timestamp [40];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (datetime,80,"%a, %d %b %Y %H:%M:%S %z",timeinfo);
    strftime (timestamp,40,"%H:%M:%S",timeinfo);
    if (conf->itest) {
        printf("Email start: %s\n",datetime);
    }
    //
    char data [50] ;
    time_t throttletime = rawtime - 3600;   
    char *maillog   = "/var/gco/mailllog.txt" ;
    FILE *fpconf ;
    fpconf = fopen( maillog, "r" ) ;
    int mailcount = 0 ;
    if ( fpconf != NULL ){
        while( fgets ( data, 50, fpconf ) != NULL ){
            //if (conf->itest) printf("log data  %s",data);
            if (istimestamp(data)) {
                //if (conf->itest) printf("mail log data  %s\n",data);
                long timestamp = atol(data) ;
                if (timestamp > throttletime) {
                    mailcount++ ;
                }
            }
        }
        fclose(fpconf);
    }
    if (mailcount > conf->imaillimit) {
        logmsg("email suppressed - anti floooding") ;
        return 0 ;
    }
    //
    //
    char payload_text [5000];
    int i ;
    strcpy(payload_text,"Date: ") ;
    strcat(payload_text,datetime) ;
    strcat(payload_text,"\r\n") ;
    //
    for ( i = 0; i < conf->ito; i++) {
        if (i == 0) {
            strcat(payload_text,"To: ") ;
        }  else {
            strcat(payload_text, " , ") ;
        }
        strcat(payload_text,conf->sztoperson[i]);
        strcat(payload_text," ") ;
        strcat(payload_text,conf->szsmtpto[i]);
    }
    //
    strcat(payload_text,"\r\nFrom: ") ;
    strcat(payload_text,conf->szfromperson);
    strcat(payload_text," ") ;
    strcat(payload_text,conf->szsmtpfrom);
    //
    for ( i = 0; i < conf->icc; i++) {
        if (i == 0) {
            strcat(payload_text,"\r\nCc: ") ;
        }  else {
            strcat(payload_text, " , ") ;
        }
        strcat(payload_text,conf->szccperson[i]);
        strcat(payload_text," ") ;
        strcat(payload_text,conf->szsmtpcc[i]);
    }
    for ( i = 0; i < conf->ibcc; i++) {
        if (i == 0) {
            strcat(payload_text,"\r\nBcc: ") ;
        }  else {
            strcat(payload_text, " , ") ;
        }
        strcat(payload_text,conf->szbccperson[i]);
        strcat(payload_text," ") ;
        strcat(payload_text,conf->szsmtpbcc[i]);
    }

    const char* domain = "aussiesky.net";     // Change to your domain
    char* msgid = generate_message_id(domain);
    printf("Message-ID: %s\n", msgid);
    strcat(payload_text,"\r\nMessage-ID: ") ;
    strcat(payload_text,msgid) ;
    strcat(payload_text,"\r\n") ;
    //strcat(payload_text,"X-Priority: 3 (Normal)\r\nX-MSMail-Priority: Normal\r\n") ;
    strcat(payload_text,"Subject: ") ;
    if (conf->roofstatus) {
        strcat(payload_text,"Observatory Roof has been Opened");
    } else {
        strcat(payload_text,"Observatory Roof has been Closed");
    }
    strcat(payload_text," ");
    strcat(payload_text,timestamp);
    strcat(payload_text,"\r\n\r\n") ;
    if (conf->roofstatus) {
        strcat(payload_text,"Event: Roof has been Opened");
    } else {
        strcat(payload_text,"Event: Roof has been Closed");
    }
    strcat(payload_text,"\r\n\r\n") ;
    strcat(payload_text,"Timestamp: ") ;
    strcat(payload_text,datetime);
    strcat(payload_text,"\r\n") ;
    if (strlen(conf->szaddress) > 4) {
        strcat(payload_text,"\r\nAddress: " ) ;
        strcat(payload_text,conf->szaddress ) ;
    }
    if (mailcount > conf->imaillimit -1 ) {
        sprintf(message, "\r\n\r\nN.B. further emails may be suppressed (max of %d per hour)",conf->imaillimit);
        strcat(payload_text,message) ;
        strcat(payload_text,"\r\nCheck roof activity here --> https://www.aussiesky.net/GCOMiddle/rooflog.html") ;
    }
    strcat(payload_text,"\r\n") ;
        if (conf->itest) {
        printf("Payload: \n%s\n",payload_text); 
    }
    //
    // 2. Open pipe to Postfix's sendmail binary
    // The -t flag tells sendmail to extract recipients from the To: header
    FILE *mail_pipe = popen("/usr/sbin/sendmail -t -f noreply@aussiesky.net", "w");
    if (mail_pipe == NULL) {
        logmsg("failed to open sendmail pipe");
        return 1;
    }
    // 3. Write headers and body to the pipe
    fprintf(mail_pipe, "%s\n", payload_text);
    // 4. Close the pipe and verify Postfix accepted the email
    int exit_code = pclose(mail_pipe);
    if (exit_code == 0) {
        logmsg("email successfully queued in postfix");
        //
        time_t now;
        now = time(0);
        char buffer[30] ;
        FILE *mail = fopen(maillog, "a");
        if (!mail) {
            return -2;
        }
        sprintf(buffer,"%lld\n",now) ;
        fprintf(mail,buffer);
        fclose(mail);   
        //
    } else {
        sprintf(message, "error queueing email. Postfix exit code: %d", exit_code);
        logmsg(message) ;
    }
    return 0;
}
