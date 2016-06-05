#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <openssl/md5.h>
#define NGX_HTTP_CACHE_KEY_LEN       16
#define NGX_HTTP_CACHE_ETAG_LEN      42
#define NGX_HTTP_CACHE_VARY_LEN      42
/*
   author: liuguirong
   date: 2016.3.6
*/
/*gcc -o get_cache_header -g get_nginx_file_cache_header.c -lcrypto -lssl*/
const char * const options_short_long = "k:d:f:vh";
const struct option long_options[] = { 
	{"dir", 1, NULL, 'd'},
	{"help", 0, NULL, 'h'},
	{"version", 0, NULL, 'v'},
	{"file", 1, NULL, 'f'},
	{"key", 1, NULL, 'k'},
	{NULL, 0 , NULL, 0}
};
const char *version = "0.0.1";
extern char *optarg;

typedef uintptr_t       ngx_uint_t;

typedef struct {
    ngx_uint_t                       version;
    time_t                           valid_sec;
    time_t                           last_modified;
    time_t                           date;
    uint32_t                         crc32;
    u_short                          valid_msec;
    u_short                          header_start;
    u_short                          body_start;
    u_short                          purge_directory;
} ngx_http_file_cache_header_t;


typedef struct {
    ngx_uint_t                       version;
    time_t                           valid_sec;
    time_t                           last_modified;
    time_t                           date;
    uint32_t                         crc32;
    u_short                          valid_msec;
    u_short                          header_start;
    u_short                          body_start;
    u_char                           etag_len;
    u_char                           etag[NGX_HTTP_CACHE_ETAG_LEN];
    u_char                           vary_len;
    u_char                           vary[NGX_HTTP_CACHE_VARY_LEN];
    u_char                           variant[NGX_HTTP_CACHE_KEY_LEN];
} ngx_http_file_cache_header_197_t;

char *human_ts(time_t *tt) {
      struct tm *st = localtime(tt);
      static char good_ts[32];
      strftime(good_ts, 32, "%c", st);
      return good_ts;
}

int mystr_tolower(char *mystr) {
   if (!mystr)
      return 0;
   char *p = mystr;
   while( *p) {
       if (*p >= 'A' && *p <= 'Z') {
           *p = *p + 32;
       }
       p++;
   }
   return 0;
}

u_char *
ngx_hex_dump(u_char *dst, u_char *src, size_t len)
{
    static u_char  hex[] = "0123456789abcdef";

    while (len--) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0xf];
    }

    return dst;
}

int md5_calcu(char *md5_value, const char *key, int len) {
    MD5_CTX md5; 
    MD5_Init(&md5);
    MD5_Update(&md5, key, len);
    MD5_Final(md5_value, &md5);
    return 0;
}
int show_help() {
   printf("usage:\n");
   printf("\tget_cache_header  [-f read header direct from file, not using key] [-k key if not given -f] [-d cache_path default value: /usr/local/nginx/data/cache]\n");
   printf("\tignore -k option if given both -f option and -k option\n");
   return 0;
}
int main(int argc, char *argv[]) {

    //printf("126 size:%d, 197 size:%d\n", sizeof(ngx_http_file_cache_header_t), sizeof(ngx_http_file_cache_header_197_t));
    if (argc < 2) {
        show_help();
        return 1;
    }
    char ch;
    char cache_path[256] = {'\0'};
    char cache_file[512] = {'\0'};
    char cache_key[512] = {'\0'};
    while((ch = getopt_long(argc, argv, options_short_long, long_options, NULL)) != -1) {
	    switch(ch) {  
		    case 'd':            
			    strcpy(cache_path, optarg);
			    break;                      
		    case 'h':            
			    show_help();
			    return 0; 
		    case 'v':            
			    printf("%s\n", version);
			    return 0; 
		    case 'f':
			    strcpy(cache_file, optarg);
			    break;
		    case 'k':
			    strcpy(cache_key, optarg);
			    break;
		    case '?':             
		            printf("parameter error\n");
		            show_help();
		return 1;
	    }             
    } 
    if (strlen(cache_file) == 0 && strlen(cache_key) == 0) {
       printf("-k or -f option must specified one\n");
       return 1;
    }
    /*using default value*/
    if (strlen(cache_path) == 0) {
        strcpy(cache_path, "/usr/local/nginx/data/cache");
    }

    /*using key to calcu filename*/
    if (strlen(cache_file) == 0 ) {
        char filename[512];
        char md5[256];
        char dst[256];
        printf("key:%s\n", cache_key);
        memset(md5, 0, 256);
        memset(dst, 0, 256);
        mystr_tolower(cache_key);
        md5_calcu(md5, cache_key, strlen(cache_key)); 
        char *p = ngx_hex_dump(dst, md5, 16);
        *p = '\0';
        char last_2_bit[3]; 
        char last_sec_2_bit[3]; 
        sprintf(last_2_bit, "%s", dst + strlen(dst) - 2);
        snprintf(last_sec_2_bit, 3, "%s", dst + strlen(dst) - 4);
        sprintf(filename, "%s/%s/%s/%s", cache_path, last_2_bit, last_sec_2_bit, dst); 
        printf("md5:%s\n", dst);
        strcpy(cache_file, filename);
    }
    printf("filename:%s\n", cache_file);
    FILE *fp = fopen(cache_file, "r");
    if (!fp) {
        printf("open %s failed, make sure it exists!\n", cache_file);
        return -1;
    }
    ngx_http_file_cache_header_t data_buff;
    memset(&data_buff, 0, sizeof(ngx_http_file_cache_header_t)); 
    int len = fread(&data_buff, 1, sizeof(ngx_http_file_cache_header_t), fp);
    if (len == 0) {
        printf("read failed\n");
        return -1;
    }
    fclose(fp);
    printf("version:%u\n", data_buff.version);
    printf("valid_sec:%u, %s\n", data_buff.valid_sec, human_ts(&data_buff.valid_sec));
    printf("last_modified:%u, %s\n", data_buff.last_modified, human_ts(&data_buff.last_modified));
    printf("date:%u, %s\n", data_buff.date, human_ts(&data_buff.date));
    printf("cache time span:%u\n", data_buff.valid_sec - data_buff.date);
    printf("crc32:%u\n", data_buff.crc32);
    printf("valid_msec:%d\n", data_buff.valid_msec);
    printf("header_start:%d\n", data_buff.header_start);
    printf("body_start:%d\n", data_buff.body_start);

    return 0;
}
