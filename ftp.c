#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
//#include <syslog.h>
#include <unistd.h>
#include <roofcheck.h>
//
int logmsg(char* message);
//


struct upload_data {
    FILE *fp;
    size_t remaining;
};

/* Read callback for uploading from file */
static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct upload_data *ud = (struct upload_data *)userdata;
    size_t max_to_read = size * nmemb;
    
    if (max_to_read > ud->remaining)
        max_to_read = ud->remaining;
    
    if (max_to_read == 0)
        return 0;
    
    size_t bytes_read = fread(ptr, 1, max_to_read, ud->fp);
    ud->remaining -= bytes_read;
    return bytes_read;
}

int upload_file(CURL *curl, const char *sourcedir, const char *local_path, const char *remote_url)
{
    char local_path_full [30] ;
    strcpy(local_path_full, sourcedir) ;
    strcat(local_path_full, local_path) ;
    FILE *fp = fopen(local_path_full, "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open local file %s\n", local_path_full);
        return 1;
    }
    CURLcode res;
    fseek(fp, 0, SEEK_END);
    curl_off_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    struct upload_data ud = { fp, (size_t)fsize };

    curl_easy_setopt(curl, CURLOPT_URL, remote_url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, &ud);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fsize);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);  // Create directories if needed
    //
    for (int i = 1; i <= 3 &&
        (res = curl_easy_perform(curl)) != CURLE_OK; i++) {
        sleep(1);
    }
    fclose(fp);
    if (res != CURLE_OK) {
        //printf("Upload of %s failed\n", remote_url);
        return 1;
    } else {
        //printf("Uploaded %s successfully\n", remote_url);
        return 0;
    }
}

int ftp_send(struct configparms * conf)
{
    char message[600] ;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init() failed\n");
        curl_global_cleanup();
        return 1;
    }
	//
    char full_url[500];
    char userpwd[256];
	//
    strcpy(userpwd,conf->szftpuser) ;
    strcat(userpwd,":") ;
    strcat(userpwd,conf->szftppass) ;
    //snprintf(userpwd, sizeof(userpwd), "%s:%s", conf->szftpuser, conf->szftppass);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
	//
    for (int i = 0 ;  i < conf->ifiles ; i++) {
        strcpy(full_url,"ftp://") ;
        strcat(full_url,conf->szftpurl) ;
        strcat(full_url,conf->szftpfile[i]);
        if (upload_file(curl, conf->szftpsource, conf->szftpfile[i], full_url)) {
            sprintf(message,"upload to %s failed", full_url);
            logmsg(message) ;
        } else {
            sprintf(message, "upload to %s OK", full_url);
            logmsg(message) ;
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}