
#include "header.h"

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif 

//macros for styling 
#define RESET       "\033[0m"
#define BOLD        "\033[1m"

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




//create a TCP socket , 
static int make_listen_socket(uint16_t port)
    {
    //make a socket
    /*AF_INET = specify the address family IPv4
    SOCK_STREAM = specify the socket type as stream socket
    0= means sys choose appropiate protocal (tcp for SOCK_STREAM)*/
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
        perror("Socket");
        exit(1);
        }
    int one = 1;

    /*setsockopt = sets socket options
    SOL_SOCKET = level at which this options are define (for socket level options)
    SO_REUSEADDR = search online (ye thoda dificult hai)*/
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
        perror("setsockopt");
        exit(1);
        }

    /*struct sockaddr_in = structure to store the socket addres
    info for IPv4 like address family , port and ip address*/
    struct sockaddr_in addr;

    //memset is used to initalize the addr structure to zero 
    memset(&addr, 0, sizeof(addr));

    /*
* addr.sin_addr.s_addr: The IPv4 address for the socket.
* INADDR_ANY: A symbolic constant that represents any available
network interface address on the host. This means the server will
listen on all network interfaces.
* htonl(): Host to Network Long. Converts a 32-bit unsigned integer
from host byte order to network byte order. Network byte order is big-endian.

* addr.sin_port: The port number for the socket.
* htons(): Host to Network Short. Converts a 16-bit unsigned integer (port number) from host byte order to network byte order.*/
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    /*binding the  socket to the port
    -bind = it assigns the local address and port to the socket*/
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
        perror("[Error] Bind");
        exit(1);
        }

    //putting the socket into listening mode (accept connections)
    /*listen = puts the socket into listening state so that it can
    accept incoming connection requests*/
    if (listen(fd, SOMAXCONN) < 0) {
        fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
        perror("[Error] listen");
        exit(1);
        }
    return fd;
    }


int main(int argc, char* argv[])
    {
    //if port is not given through command line
    if (argc != 2) {
        fprintf(stderr, "%sUsage : %s <port>%s\n", FG_RED, argv[0], RESET);
        return 2;
        }

    uint16_t port = (uint16_t)atoi(argv[1]);
    int listen_fd = make_listen_socket(port);
    int clinets[FD_SETSIZE];
    
    //client time 
    time_t connect_t;
    time_t disconnect_t;
    
    //storing client data in a file 
    int cli_files[FD_SETSIZE];
    FILE* f_ptr[FD_SETSIZE];
    int file_count = 0;


    //Run server in debug mode 
    unsigned short int input;
    while (1)
        {
        printf("Run server in Debug mode [0- No 1- Normal Debug 2- Advance Debug]:\t");
        // %hu :- is a format specifier used for unsign short int  | %hd :- for sign short int 
        scanf("%hu", &input);
        if (input > 2) {
            fprintf(stderr, "[%sError%s] : Invalid option\n", FG_RED, RESET);
            continue;
            }
        break;
        }



    //initalize the client array with -1 
    for (int i = 0;i < FD_SETSIZE;i++) {
        clinets[i] = -1;
        cli_files[i] = -1;
        }
    printf("%sListening to port %u (fd=%d)\n%s", FG_BGREEN, (unsigned)port, listen_fd, RESET);
    //event loop
    while (1) {
        //sets the file descriptors in fd_set
        fd_set  rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        int maxfd = listen_fd;
        //marks the clients 
        for (int i = 0;i < FD_SETSIZE;i++)
            {
            if (clinets[i] != -1)
                {
                FD_SET(clinets[i], &rfds);
                if (clinets[i] > maxfd) {
                    maxfd = clinets[i];
                    }
                }
            }

        //time period 
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        //blocks until a fd gets ready or time interval ends
        int ready = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (ready == -1) {
            if (errno == EINTR) {
                continue;
                }
            fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
            perror("select");
            break;
            }
        if (ready == 0) {
            puts("[Timeout]");
            continue;
            }

        //new connction ,when a new client wants to connects
/*FD_ISSET: Checks if a specific file descriptor is present (set) in an fd_set.
This condition checks if the listen_fd is set in rfds,
meaning a new connection is available.*/

//why we used FD_ISSET(listen_fd, &rfds) check QUESTION.md
        if (FD_ISSET(listen_fd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t len = sizeof(cli);

            //checks multiple fd at same time and marks then the fd which are ready to read 
            //accept creates the new socket(new conncetion) for data transfer 
            int cli_fd = accept(listen_fd, (struct sockaddr*)&cli, &len);
            if (cli_fd >= 0)
                {
                connect_t = time(NULL);
                printf("\n[%sClient Conected%s] %-20s", FG_GREEN, RESET, ctime(&connect_t));

                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
                printf("%sAccepted fd = %d from %s : %d%s\n", FG_BYELLOW, cli_fd, ip, ntohs(cli.sin_port), RESET);
                //add clients
                int placed = 0;
                for (int i = 0; i < FD_SETSIZE;i++) {
                    if (clinets[i] == -1) {
                        clinets[i] = cli_fd;
                        placed = 1;
                        cli_files[i] = cli_fd;
                        char addr_buf[30];
                        snprintf(addr_buf, sizeof(addr_buf), "client_files/%s", ip);
                        char*cli_directory_path = strdup(addr_buf);
                        if (create_directory(addr_buf) == -1)
                            {
                            exit(-1);
                            }
                        snprintf(addr_buf, sizeof(addr_buf), "%s/cli_%d.txt", cli_directory_path, file_count++);
                        free(cli_directory_path);
                        f_ptr[i] = fopen(addr_buf, "w");
                        break;
                        }
                    }

                //if max client limit reached
                if (!placed) {
                    fprintf(stderr, "%sToo many clients; closing fd=%d%s", FG_RED, cli_fd, RESET);
                    close(cli_fd);
                    }
                //EINTR means , sys call interrupted by signal
                else if (errno != EINTR)
                    {
                    perror("Accept");
                    }
                }

            }

        for (int i = 0;i < FD_SETSIZE;i++) {
            int fd = clinets[i];
            int file_number = i;
            if (fd == -1) {
                continue;
                }
            if (!FD_ISSET(fd, &rfds)) {
                continue;
                }
            // In your read section, add detailed debugging:
            char buf[4096];
            ssize_t n = read(fd, buf, sizeof(buf));

            //debug code , tells actually what we are recived from client in hexhump format
            if (input == 1)
                {
                // %zd :- format specifer  for ssize_t
                printf("%sREaded %zd bytes  | from fd=%d%s\n ", FG_BBLUE, n, fd, RESET);
                fwrite(buf, 1, n, stdout);
                }

            else if (input == 2) {
                printf("%s=== READ DEBUG ===%s\n", FG_CYAN, RESET);
                printf("Read returned: %zd bytes\n", n);
                printf("errno: %d (%s)\n", errno, strerror(errno));

                if (n > 0) {
                    printf("Raw data (%zd bytes):\n", n);

                    // Show each byte in hex + char
                    for (ssize_t i = 0; i < n; i++) {
                        unsigned char c = (unsigned char)buf[i];
                        printf("%02x ", c);
                        if ((i + 1) % 16 == 0 || i == n - 1) {
                            // Pad and show characters
                            for (ssize_t j = (i / 16) * 16; j <= i; j++) {
                                unsigned char ch = (unsigned char)buf[j];
                                printf("%c", (ch >= 32 && ch <= 126) ? ch : '.');
                                }
                            printf("\n");
                            }
                        }
                    printf("String representation:\n");
                    fwrite(buf, 1, n, stdout);
                    printf("\n%s=== END DEBUG ===%s\n\n", FG_CYAN, RESET);
                    }
                }


            if (n <= 0) {
                if (n < 0) {
                    fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
                    perror("read");
                    }
                else {
                    disconnect_t = time(NULL);
                    printf("\n[%sClinet Disconnected%s] %s", FG_RED, RESET, ctime(&disconnect_t));
                    }
                close(fd);
                clinets[i] = -1;
                cli_files[i] = -1;
                fclose(f_ptr[i]);
                continue;
                }

            //echo write back
            write(fd, FG_BYELLOW, (size_t)sizeof(FG_BYELLOW));

            
            if (n > 0 && n < (ssize_t)sizeof(buf)) {
                buf[n] = '\0';  // Add null terminator
            }
            fprintf(f_ptr[file_number],"%s", buf);
            // fwrite(buf, 1, n, f_ptr[file_number]);

            ssize_t m = write(fd, buf, (size_t)n);
            write(fd, RESET, (size_t)sizeof(RESET));
            if (m < 0) {
                fprintf(stderr, "[%sError%s]", FG_BRED, RESET);
                perror("Write");
                close(fd);
                disconnect_t = time(NULL);
                printf("\n[%sClinet Disconnected%s] %s", FG_RED, RESET, ctime(&disconnect_t));
                clinets[i] = -1;
                }
            }
        }
    //closing listening socket
    close(listen_fd);
    return 0;
    }