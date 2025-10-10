
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

// Style macros
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define ITALIC      "\033[3m"
#define UNDERLINE   "\033[4m"
#define BLINK       "\033[5m"
#define REVERSE     "\033[7m"
#define HIDDEN      "\033[8m"
#define STRIKE      "\033[9m"

// Foreground color macros
#define FG_BLACK    "\033[30m"
#define FG_RED      "\033[31m"
#define FG_GREEN    "\033[32m"
#define FG_YELLOW   "\033[33m"
#define FG_BLUE     "\033[34m"
#define FG_MAGENTA  "\033[35m"
#define FG_CYAN     "\033[36m"
#define FG_WHITE    "\033[37m"

// Bright foreground colors
#define FG_BBLACK   "\033[90m"
#define FG_BRED     "\033[91m"
#define FG_BGREEN   "\033[92m"
#define FG_BYELLOW  "\033[93m"
#define FG_BBLUE    "\033[94m"
#define FG_BMAGENTA "\033[95m"
#define FG_BCYAN    "\033[96m"
#define FG_BWHITE   "\033[97m"

// Background colors
#define BG_BLACK    "\033[40m"
#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"
#define BG_WHITE    "\033[47m"

// Bright background colors
#define BG_BBLACK    "\033[100m"
#define BG_BRED      "\033[101m"
#define BG_BGREEN    "\033[102m"
#define BG_BYELLOW   "\033[103m"
#define BG_BBLUE     "\033[104m"
#define BG_BMAGENTA  "\033[105m"
#define BG_BCYAN     "\033[106m"
#define BG_BWHITE    "\033[107m"

// custom macros
#define MSG_SEPRATE "!?!?"
#define MSG_SEP_LEN strlen(MSG_SEPRATE)
#define META_BUFFER_SIZE 256

/*
Meanings of return in directory_create_function :
    return -1 : Error
    return 0  : Success [ directory Already Exist]
    return 1  : Success [ Directory Created]
*/

int create_directory(const char* path,int debug)
    {
    struct stat st = { 0 };

    // Check if directory already exists
    if (stat(path, &st) == -1) {
        // Directory doesn't exist, create it
        if (mkdir(path, 0755) == -1) {
            fprintf(stderr, "Error creating directory '%s': %s", path, strerror(errno));
            return -1;
            }
        printf("Directory '%s' created successfully.", path);
        return 1; // Created new directory
        }
    else {
        // Check if it's actually a directory
        if (S_ISDIR(st.st_mode)) {
            if (debug == 3) {
                printf("Directory '%s' already exists.", path);
                }
            return 0; // Directory already exists
            }
        else {
            fprintf(stderr, "Error: '%s' exists but is not a directory.", path);
            return -1;
            }
        }
    }

typedef struct client_info {
    char* cli_name;
    char* cli_uuid;
    int cli_id;
    }client_info;

// random number generator
int random_int()
    {
    return (int)(rand() % 7);
    }

bool combine_msg(char* msg, char* new_msg)
    {
    // sequence : 1.server name is already present in msg
    // 2.MSG_SEPERATE + clolor_code
    /* BUGs:-
it is apppending the color code of , when each time clinet connect. fix this
    */
    int size_dest = strlen(msg), size_src = strlen(new_msg);

    if (size_dest + size_src + 1 < META_BUFFER_SIZE) {
        strcat(msg, MSG_SEPRATE);
        strcat(msg, new_msg);
        printf("%s\n", msg);

        return true;
        }
    //Final sequence : server name + MSG_SEPERATE + color_code 
    return false;
    }

// break meta_data_msg into particular sub_msg

bool break_meta_d(client_info** cli__, char* buffer)
    {
    char* ptr;
    short int size;
    if ((ptr = strstr(buffer, MSG_SEPRATE)) != NULL) {
        size = ptr - buffer;
        // 1.name filling
        (*cli__)->cli_name = strndup(buffer, size);
        ptr = ptr + MSG_SEP_LEN;
        // 2. color_code filling
        (*cli__)->cli_uuid = strdup(ptr);
        return true;
        }
    return false;
    }

// refresh the buffer [erase info except server name]
void meta_buffer_refresh(char* buffer, int len)
    {
    buffer[len] = '\0';
    }

// free the client info when client disconnects
void close_client(void* cli_,int fd, int debug)
    {
    client_info* clinet = (client_info*)cli_;
    close(fd);
    free(clinet->cli_name);
    free(clinet->cli_uuid);
    free(clinet);
    (debug == 3) ? puts("freed clinet info ") : (puts(""));
    }

