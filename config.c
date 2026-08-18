//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <roofcheck.h>
#include <regex.h>
#include <ctype.h>
//
//#define MAXADDR 5
int logmsg(char* message) ;
//
//
char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}//
int config(struct configparms *conf ) {
    strcpy(conf->szaddress,"  ") ;
    strcpy(conf->szsmtpsubj,"Email subject") ;
    strcpy(conf->szsmtpfrom,"") ;
    strcpy(conf->szftpurl,"") ;
    strcpy(conf->szftpuser,"") ;
    strcpy(conf->szftppass,"") ;
    conf->debounce = 11 ;
    conf->notify = 1 ;
    conf->imaillimit = 6 ;
    //regex_t rxfilename;
    regex_t rxsmtpto ;
    regex_t rxsmtpfrom ;
    regex_t rxsmtpcc ;
    regex_t rxsmtpbcc ;
    regex_t rxsmtpsubj ;
    regex_t rxaddress;
    regex_t rxftpurl;
    regex_t rxftpsource;
    regex_t rxftpfile;
    regex_t rxftpuser;
    regex_t rxftppass;
    regex_t rxdebounce;
    regex_t rxmaxemail;
    regex_t rxnotify;
    //  
    char *rzsmtpto     = "^smtpto\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    //char *rzsmtpto     = "^smtpto\\s+(.*)$" ;
    char *rzsmtpfrom   = "^smtpfrom\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    char *rzsmtpcc     = "^smtpcc\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    char *rzsmtpbcc    = "^smtpbcc\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    char *rzsmtpsubj   = "^smtpsubj\\s+([A-Za-z0-9_ ]+)\\s.$" ;
    char *rzaddress    = "^address\\s+(.*)$" ;
    char *rzftpurl     = "^ftpurl\\s+(.*)$" ;
    char *rzftpsource  = "^ftpdir\\s+(.*)$" ;
    char *rzftpfile    = "^ftpfile\\s+(.*)$" ;
    char *rzftpuser    = "^ftpuser\\s+(.*)$" ;
    char *rzftppass    = "^ftppass\\s+(.*)$" ;
    char *rzdebounce   = "^debounce\\s+([0-9]+)" ;
    char *rzmaxemail   = "^emaillimit\\s+([0-9]+)" ;
    char *rznotify     = "^notify\\s+(.*)$" ;
    //
    regcomp(&rxsmtpto,    rzsmtpto, REG_EXTENDED);
    regcomp(&rxsmtpfrom,  rzsmtpfrom, REG_EXTENDED);
    regcomp(&rxsmtpcc,    rzsmtpcc, REG_EXTENDED);
    regcomp(&rxsmtpbcc,   rzsmtpbcc, REG_EXTENDED);
    regcomp(&rxsmtpsubj,  rzsmtpsubj, REG_EXTENDED);
    regcomp(&rxaddress,   rzaddress, REG_EXTENDED);
    regcomp(&rxftpurl,    rzftpurl, REG_EXTENDED);
    regcomp(&rxftpsource, rzftpsource, REG_EXTENDED);
    regcomp(&rxftpfile,   rzftpfile, REG_EXTENDED);
    regcomp(&rxftpuser,   rzftpuser, REG_EXTENDED);
    regcomp(&rxftppass,   rzftppass, REG_EXTENDED);
    regcomp(&rxdebounce,  rzdebounce, REG_EXTENDED);
    regcomp(&rxmaxemail,  rzmaxemail, REG_EXTENDED);
    regcomp(&rxnotify,    rznotify, REG_EXTENDED);
    //
    char szdebounce[10] ;
    char szmaxemail[10] ;
    char sznotify[4];
    strcpy(sznotify,"yes") ;
    size_t maxGroups = 3;
    regmatch_t groupArray[maxGroups];
    FILE *fpconf ;
    char data[200] ;
    //printf( "Opening the file test.c in read mode\n" ) ;
    fpconf = fopen( "/etc/roofcheck.conf", "r" ) ;
    if ( fpconf == NULL ){
        //printf( "Could not open file /etc/roofcheck.conf\n" ) ;
        logmsg("could not open file /etc/roofcheck.conf");
        return 1;
    }
    conf->ito = 0, conf->icc = 0, conf->ibcc = 0, conf->ifiles = 0 ;
    while( fgets ( data, 200, fpconf ) != NULL ){
        if ((!regexec(&rxsmtpto, data, maxGroups, groupArray, 0))  && (conf->ito < MAXADDR)){
            sprintf(conf->sztoperson[conf->ito],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpto[conf->ito],"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]);
            rtrim(conf->szsmtpto[conf->ito]);
            conf->ito++;
        }
        if (!regexec(&rxsmtpfrom, data, maxGroups, groupArray, 0)){
            sprintf(conf->szfromperson,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpfrom,"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]); 
            rtrim(conf->szsmtpfrom);            
        }
        if ((!regexec(&rxsmtpcc, data, maxGroups, groupArray, 0))  && (conf->icc < MAXADDR)){
            sprintf(conf->szccperson[conf->icc],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpcc[conf->icc],"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]);    
            rtrim(conf->szsmtpcc[conf->icc++]);         
        }
        if ((!regexec(&rxsmtpbcc, data, maxGroups, groupArray, 0))   && (conf->ibcc < MAXADDR)){
            sprintf(conf->szbccperson[conf->ibcc],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpbcc[conf->ibcc],"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]);
            rtrim(conf->szsmtpbcc[conf->ibcc++]);           
        }
        if (!regexec(&rxsmtpsubj, data, maxGroups, groupArray, 0)){
            sprintf(conf->szsmtpsubj,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szsmtpsubj);
        }
        if (!regexec(&rxaddress, data, maxGroups, groupArray, 0)){
            sprintf(conf->szaddress,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szaddress);
        }
        if (!regexec(&rxftpurl, data, maxGroups, groupArray, 0)){
            sprintf(conf->szftpurl,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szftpurl);
        }

        if (!regexec(&rxftpsource, data, maxGroups, groupArray, 0)){
            sprintf(conf->szftpsource,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szftpsource);
        }
        if ((!regexec(&rxftpfile, data, maxGroups, groupArray, 0))   && (conf->ifiles < MAXFILES)){
            sprintf(conf->szftpfile[conf->ifiles],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szftpfile[conf->ifiles++]); 
        }           
        if (!regexec(&rxftpuser, data, maxGroups, groupArray, 0)){
            sprintf(conf->szftpuser,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szftpuser);
        }       
        if (!regexec(&rxftppass, data, maxGroups, groupArray, 0)){
            sprintf(conf->szftppass,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szftppass);
        }
        if (!regexec(&rxdebounce, data, maxGroups, groupArray, 0)){
            sprintf(szdebounce,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(szdebounce);
            conf->debounce=atoi(szdebounce) ;
        }
         if (!regexec(&rxmaxemail, data, maxGroups, groupArray, 0)){
            sprintf(szmaxemail,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(szmaxemail);
            conf->imaillimit=atoi(szmaxemail) ;
        }
        if (!regexec(&rxnotify, data, maxGroups, groupArray, 0)){
            sprintf(sznotify,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(sznotify);
            if (strcmp(sznotify,"no")){
                conf->notify = 1 ;
            } else  {
                conf->notify = 0 ;
            }
        }
    }
    fclose(fpconf) ;
    regfree(&rxaddress);
    regfree(&rxsmtpsubj) ;
    regfree(&rxsmtpto) ;
    regfree(&rxsmtpfrom) ;
    regfree(&rxsmtpcc) ;
    regfree(&rxsmtpbcc) ;
    regfree(&rxftpurl) ;
    regfree(&rxftpsource) ;
    regfree(&rxftpfile) ;
    regfree(&rxftpuser) ;
    regfree(&rxftppass) ;
    regfree(&rxdebounce) ;
    regfree(&rxmaxemail) ;
    regfree(&rxnotify) ;
    //
    char message [500] ;
    int i ;
    sprintf(message, "From:       %s %s",conf->szfromperson,conf->szsmtpfrom) ;
    logmsg(message) ;
    for(i = 0; i < conf->ito; ++i){
        sprintf(message, "To:         %s %s", conf->sztoperson[i], conf->szsmtpto[i]) ;
        logmsg(message) ;
    }
    for(i = 0; i < conf->icc; ++i){ 
        sprintf(message, "Cc:         %s %s",conf->szccperson[i], conf->szsmtpcc[i]) ;
        logmsg(message) ;
    }
    for(i = 0; i < conf->ibcc; ++i){
        sprintf(message, "Bcc:        %s %s",conf->szbccperson[i],conf->szsmtpbcc[i]) ;
        logmsg(message) ;
    }
    sprintf(message, "Subject:    %s",conf->szsmtpsubj) ;
    logmsg(message) ;
    sprintf(message, "Address:    %s",conf->szaddress) ;
    logmsg(message) ;
    sprintf(message, "FTP URL:    %s",conf->szftpurl) ;
    logmsg(message) ;
    sprintf(message, "FTP dir:    %s",conf->szftpsource) ;
    logmsg(message) ;
    sprintf(message, "Debounce:   %d seconds",conf->debounce) ;
    logmsg(message) ;
    sprintf(message, "Notify:     %s",sznotify) ;
    logmsg(message) ;   
    sprintf(message, "Mail limit: %d emails per hour",conf->imaillimit) ;
    logmsg(message) ;    
    for(i = 0; i < conf->ifiles; ++i){
         sprintf(message, "FTP file:   %s",conf->szftpfile[i]) ;
         logmsg(message) ;
    }    
    sprintf(message, "FTP user:   %s",conf->szftpuser) ;
    logmsg(message) ;
    sprintf(message, "FTP pass:   %s",conf->szftppass) ;
    logmsg(message) ;
    return 0 ;
}
