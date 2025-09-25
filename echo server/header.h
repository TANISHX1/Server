
#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdbool.h>

/*
Meanings of return in directory_create_function :
    return -1 : Error
    return 0  : Success [ directory Already Exist]
    return 1  : Success [ Directory Created]
*/

int create_directory(const char* path)
    {
    struct stat st = {0};
    
    // Check if directory already exists
    if (stat(path, &st) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(path, 0755) == -1) {
            fprintf(stderr, "Error creating directory '%s': %s", path, strerror(errno));
            return -1;
        }
        printf("Directory '%s' created successfully.", path);
        return 1; // Created new directory
    } else {
        // Check if it's actually a directory
        if (S_ISDIR(st.st_mode)) {
            printf("Directory '%s' already exists.", path);
            return 0; // Directory already exists
        } else {
            fprintf(stderr, "Error: '%s' exists but is not a directory.", path);
            return -1;
        }
    }
}
