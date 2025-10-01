#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

// custom Macros
#define MSG_SEPRATE "!?!?"
#define MSG_SEPRATE_LEN strlen(MSG_SEPRATE)
#define META_D_BUFFER 256

// data structures
typedef struct client_info {
    char* client_name;
    char* server_name;
    char* cli_display_color;

    }client_info;

static int send_all(int sock, const void* buf, size_t len)
    {
    const char* p = (const char*)buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n < 0) {
            // interrupted by signal
            if (errno == EINTR)continue;
            // other error
            return -1;
            }

        if (n == 0) {
            break;
            }
        total += (size_t)n;
        }
    return (int)total;
    }

// check quit
bool quit_check(char* line)
    {
    return (strcmp(line, "quit\n") == 0);
    }

// display color for client
void color_pick(client_info** cli_, int num)
    {
    switch (num)
    {
        case 0:
            (*cli_)->cli_display_color = strdup(FG_BBLUE);
            break;
    
        case 1:
            (*cli_)->cli_display_color = strdup(FG_BCYAN);
            break;
    
        case 2:
            (*cli_)->cli_display_color = strdup(FG_BGREEN);
            break;
    
        case 3:
            (*cli_)->cli_display_color = strdup(FG_BMAGENTA);
            break;
    
        case 4:
            (*cli_)->cli_display_color = strdup(FG_BRED);
            break;
    
        case 5:
            (*cli_)->cli_display_color = strdup(FG_BWHITE);
            break;
    
        case 6:
            (*cli_)->cli_display_color = strdup(FG_BYELLOW);
            break;
    
        default:
            (*cli_)->cli_display_color = strdup(FG_BBLACK);
            break;
    }
    }

    // break meta_data_msg into particular sub_msg

bool break_meta_d(client_info ** cli__, char* buffer)
    {
    char* ptr;
    short int size;
    if (ptr = strstr(buffer, MSG_SEPRATE)) {
        size = ptr - buffer;
        // 1.name filling
        (*cli__)->server_name = strndup(buffer, size);
        ptr = ptr + MSG_SEPRATE_LEN;
        // 2. color_code filling
        int c_code = *ptr - '0';
        color_pick(&(*cli__), c_code);
        return true;
        }
    return false;
    }