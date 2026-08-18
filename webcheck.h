#include <time.h>
struct configparms {
    int  itest ;
    int  webstatus ;
    time_t failtime ;
    time_t resettime ;
    time_t lastemail ;
    int  delay ;
    int  interval ;
    int  ito ;
    int  icc;
    int  ibcc;
    char szpingurl[100];
    char szsmtpfrom[100];    
    char szfromperson[100] ;    
    char szsmtpto[5][100];
    char sztoperson[5][100] ;    
    char szsmtpcc[5][100] ;
    char szccperson[5][100] ;
    char szsmtpbcc[5][100] ;
    char szbccperson[5][100] ;    
};

#define MAXADDR 5


