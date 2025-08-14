#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#define _POSIX_C_SOURCE 200809L

int main(int argc, char* argv[])
    {
    bool validate_time_interval = false;
    if (argc == 2)
        {
        if (strcmp(argv[1], "YES") || strcmp(argv[1], "yes"))
            validate_time_interval = true;
        }


    if (validate_time_interval)
        puts("Enter a message (within 10 seconds)");
    else
        puts("Enter a message ");
    char buffer[256];
    while (1)
        {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int __ndfs = STDIN_FILENO + 1;
        struct timeval tv;
        if (validate_time_interval)
            {
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            }
        int ready = select(__ndfs, &readfds, NULL, NULL, validate_time_interval ? &tv : NULL);
        if (ready == -1) {
            if (errno == EINTR) {
                continue;
                }
            perror("select");
            exit(1);
            }
        if (ready == 0) {
            puts("[Time out]");
            continue;
            }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (n == 0) {
                puts("EOF | Exiting....");
                break;
                }
            if (n < 0) {
                puts("[Error] : _stdin_error");
                break;
                }
            buffer[n] = '\0';
            printf("%s", buffer);
            }
        }
    return 0;
    }