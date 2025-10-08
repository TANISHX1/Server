#include "header_cli.h"


int main(int argc, char* argv[])
    {
    bool debug = false;
    if (argc == 4) {
        debug = !(strcmp(argv[3], "-D")) ? true : false;
        if (!debug) {
            puts("Invalid debug_option");
            return 1;
            }
        }
    else if (argc != 3) {

        fprintf(stderr, "%s%sUsage:%s <server_ip> <port> <debug_mode> (optional)%s\n", ITALIC, FG_RED, argv[0], RESET);
        return 1;
        }
    const char* server_ip = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "[%s Error %s] | Invalid_port__\n", FG_RED, RESET);
        return 1;
        }
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "[%s Error %s] | Failed_to_create_socket__\n", FG_RED, RESET);
        return 1;
        }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);

    // convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "[%s Error %s] | Invalid_IPv4_Address:%s__\n", FG_RED, RESET, server_ip);
        close(sock);
        return 1;
        }

    // connect to server
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[%s Error %s] | Connect\n", BG_RED, RESET);
        close(sock);
        return 1;
        }

    // client info structure filling /initilization
    client_info* client_info_t = (client_info*)malloc(sizeof(client_info));
    char buffer[META_D_BUFFER_SIZE];
    ssize_t n_byte;
    printf("%s%sChoose a name to connect server :%s\t", UNDERLINE, FG_BBLUE, RESET);
    fflush(stdout);
    fgets(buffer, sizeof(buffer), stdin);
    client_info_t->client_name = strdup(buffer);
    // to fectch or generate the uuid of client 
    client_info_t->cli_uuid = (char*)malloc(37 * sizeof(char));
    uuid_fetch(client_info_t->cli_uuid, debug);
    combine_msg(buffer, client_info_t->cli_uuid);

    // send meta data to the server 
    if (send(sock, buffer, strlen(buffer) , 0) < 0) {
        perror("send Meta Data :");
        return 1;
        }
    // recieve server info || 
    if ((n_byte = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[n_byte] = '\0';

        break_meta_d(&client_info_t, buffer);

        printf("Recieved :%s\n", client_info_t->server_name);
        printf("Recieved :%sxxxxx%s\n", client_info_t->cli_display_color, RESET);

        }
    else {
        perror("Recv Meta Data :");
        return 1;
        }




    printf("Connected to server [ IP= %s%s%s ] [ Port= %s%d%s ]\n", FG_GREEN, server_ip, RESET, FG_GREEN, port, RESET);

    // Meta data of client
    printf("%s%s\n\t\t Meta Data Of Client Sended to the server%s\n\n", FG_BBLUE, BOLD, RESET);
    printf("Server Name : %s\n", client_info_t->server_name);
    printf("Client Name : %s\n", client_info_t->client_name);
    printf("clinet uuid : %s\n", client_info_t->cli_uuid);
    // printf("MAC Address: %s\n");

    // msg sending and recieving loop
    while (1) {
        printf("%s>>>%s  ", FG_BGREEN, RESET);
        fflush(stdout);

        // line - to store the location of mem where input stored
        char* line = NULL;
        size_t cap = 0;
        ssize_t nread = getline(&line, &cap, stdin);
        if (nread == -1) {
            if (feof(stdin)) {
                printf("EOF on stdin. Exiting.\n");
                }
            else {
                fprintf(stderr, "[%s Error %s] | Getline\n", BG_RED, RESET);
                }
            free(line);
            break;
            }

        // check quit command  
        if (quit_check(line)) {
            free(line);
            break;
            }

        int send_newline = 0;


        // send the user input
        if (nread > 0) {
            if (send_all(sock, line, (size_t)nread) < 0) {
                fprintf(stderr, "[%s Error %s] | send_all\n", FG_RED, RESET);
                free(line);
                break;
                }

            if (debug) {
                printf("Sent :%zd bytes\n", send_newline ? nread + 1 : nread);
                }
            }
        free(line);

        // Receive a reply (unchanged, but ensure null-termination)
        char buffer_recv[4096];
        ssize_t recv_size = recv(sock, buffer_recv, sizeof(buffer_recv) - 1, 0);
        if (recv_size > 0) {
            buffer_recv[recv_size] = '\0';
            if (debug) {
                printf("Recieved :%zd bytes\n", recv_size);
                }
            printf("%s\n", buffer_recv);
            }
        else if (recv_size == 0) {
            printf("Server closed the connection.");
            break;
            }
        else {
            perror("recv");
            break;
            }
        }
    free_client(client_info_t);
    close(sock);
    return 0;
    }