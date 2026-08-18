
struct configparms {
    int  itest ;
    int  debounce ;
    int  notify ;
    int  roofstatus ;
    int  ito ;
    int  icc;
    int  ibcc;
    int  ifiles ;
    int  imaillimit;
    char szaddress[100];
    char szsmtpsubj[100];
    char szsmtpfrom[100];    
    char szfromperson[100] ;    
    char szsmtpto[5][100];
    char sztoperson[5][100] ;    
    char szsmtpcc[5][100] ;
    char szccperson[5][100] ;
    char szsmtpbcc[5][100] ;
    char szbccperson[5][100] ;    
    char szftpuser[100];
    char szftppass [100] ;
    char szftpurl[200] ;
    char szftpsource[200] ;
    char szftpfile[10][30];
};
//struct configparms conf , *conptr;
    
#define MAXADDR 5
#define MAXFILES 10




