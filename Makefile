all:
	gcc -o get_cache_header -g get_nginx_file_cache_header.c -lcrypto -lssl
