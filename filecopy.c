//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <time.h>
#include <roofcheck.h>

int copy_file(const char *src_path, const char *dest_path) {
    char timestamp [30] ;
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(timestamp,30,"%Y-%m-%d %H:%M:%S",timeinfo);    
    //
    regex_t *preg = calloc(1, sizeof(regex_t));
    int numoffsets = 2;
    regmatch_t pmatch[numoffsets];
    regcomp(preg, "^.*(xxxxxx).*$", REG_EXTENDED);
    //printf("regcomp output = %i\n", rc);
    FILE *src ;   
    src = fopen( src_path, "r" ) ;
    if ( src == NULL ){
        printf( "Could not open file %s\n",src_path ) ;
        return 1;
    }
    FILE *dest = fopen(dest_path, "w");
    if (!dest) {
        fclose(src);
        return -2;
    }    
    char buffer[2048]; 
    char subbuffer[2048];
    while( fgets ( buffer, sizeof(buffer), src ) != NULL ){
        if (!regexec(preg, buffer, numoffsets, pmatch, 0)) {
            int datestart = pmatch[1].rm_so ;
            int dateend = pmatch[1].rm_eo ;
            strncpy(subbuffer,buffer,datestart) ;
            //strcpy(subbuffer,buffer) ;
            subbuffer[datestart] = '\0';
            strcat(subbuffer,timestamp) ;
            strcat(subbuffer,&buffer[dateend]) ;
            fprintf(dest,subbuffer) ;
            //printf("%s SUBBUFFER %s\n", timestamp, subbuffer) ;
        } else {
            fprintf(dest,buffer) ;
        }
    }
    fclose(src) ;
    fclose(dest) ;
    regfree(preg);  
    return 0; // Success
}
int rooflog(int roofstate) {
    //
    // Add a record at the BEGINNING of the log file
    // i.e reveerse cronological order
    //
    char *tempfile   = "/var/gco/temp.txt" ;
    char *logfile    = "/var/gco/rooflog.txt" ;
    remove(tempfile) ;
    time_t now;
    now = time(0);
    char buffer[200] ;
    FILE *temp = fopen(tempfile, "w");
    if (!temp) {
        return -2;
    }
    sprintf(buffer,"%lld,%d\n",now,roofstate) ;
    fprintf(temp,buffer);
    //
    // Add remaider of records
    //
    FILE *log = fopen(logfile, "r");
    if (!log) {
        return -2;
    }   
    while( fgets ( buffer, sizeof(buffer), log ) != NULL ){
        fprintf(temp,buffer) ;
        //printf(buffer) ;
    }   
    fclose(log);
    fclose(temp);
    remove(logfile) ;
    rename(tempfile, logfile);
    return 0;
}
//
//
int logpage(struct configparms * conf){
    //
    char buffer[500] ;
    char headername[50];
    char listname[50];  
    strcpy(headername,conf->szftpsource) ;
    strcat(headername,"header.html") ;
    strcpy(listname,conf->szftpsource) ;
    strcat(listname,"list.html");
    FILE *list = fopen(listname, "w");
    FILE *header = fopen(headername, "r");
    while( fgets ( buffer, sizeof(buffer), header ) != NULL ){
        fprintf(list,buffer) ;
        //printf(buffer) ;
    }   
    fclose(header);
    //
    FILE *log = fopen("/var/gco/rooflog.txt", "r");
    if (!log) {
        return -2;
    }
    while( fgets ( buffer, sizeof(buffer), log ) != NULL ){
        char szstamp[12];
        strncpy(szstamp, buffer,10) ;
        szstamp[10] = '\0' ;
        //dest[n-1] = '\0';
        char szstate[10] ;
        strcpy(szstate,&buffer[11]) ;
        //printf("Stamp %s, State %s\n", szstamp, szstate);
        long stamp = atol(szstamp) ;
        long state = atol(szstate) ;
        char timestamp [22] ;
        time_t rawtime = stamp;
        struct tm * timeinfo;
        //time (&rawtime);
        timeinfo = localtime (&rawtime);
        strftime(timestamp,22,"%Y-%m-%d %H:%M:%S",timeinfo);
        if (state) {
            strcpy(szstate,"OPEN") ;
        } else {
            strcpy(szstate,"CLOSED") ;
        }
        fprintf(list,"  <tr>\n" );
        fprintf(list,"    <td width=\"7%%\"> </td>\n") ;
        fprintf(list,"    <td width=\"11%%\" class=\"verdana\"> <strong> %s </strong></td>\n", szstate) ;
        fprintf(list,"    <td width=\"82%%\" class=\"verdana\"> <strong> %s </strong></td>\n", timestamp) ;
        fprintf(list,"  </tr>\n"); 
    }
    fprintf(list,"</table>\n</body>\n</html>\n\n\n");
    fclose(list);
    fclose(log) ;
    //
    //  create  Greg's text file
    //
    char timestamp [24] ;
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(timestamp,24,"%Y-%m-%d %H:%M:%S",timeinfo);
    char *textname = "/var/gco/roof.txt" ;  
    FILE *text = fopen(textname, "w");
    if (conf->roofstatus) {
        fprintf(text,"%s","OPEN") ;
    } else {
        fprintf(text,"%s","CLOSED") ;
    }    
    fprintf(text,",%s\n",timestamp) ;
    fclose(text) ;
    return 0 ;
}

