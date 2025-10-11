#include "header_cli.h"

void display_msg_safe(const char* color, const char* message, ssize_t msg_len)
    {
    pthread_mutex_lock(&display_lck);

    // save current line buffer
    int saved_point = rl_point;
    int saved_end = rl_end;
    char* saved_line = rl_copy_text(0, rl_end);

    // clear the current input line
    rl_save_prompt();
    rl_replace_line("", 0);
    rl_redisplay();

    // displaying the incoming msg
    int clean_len = msg_len;
    while ((clean_len > 0) && (message[clean_len - 1] == '\0' ||
        message[clean_len - 1] == '\n' || message[clean_len - 1] == '\r')) {
        clean_len--;
        }
    printf("\r\033[K%sOther Client%s: %.*s\n", color, RESET, clean_len, message);
    if (msg_len > 0 && message[msg_len - 1] != '\n') {
        printf("\n");
        }

    // Restore the input line 
    rl_restore_prompt();
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    rl_end = saved_end;    //
    rl_redisplay();

    free(saved_line);
    pthread_mutex_unlock(&display_lck);
    }


void* recever_thread(void* arg)
    {
    client_info* client = (client_info*)arg;
    /*
    set scket to non-blocking mode
    fcntl() - system call to get or set file options on file descriptors
    1.tell the kernal to give the current status of the file , like r,w,x and others
    2. it takes the current flag ,adds ('|') O_NONBLOCK (option) bit. and tells kernel to apply current settings
    */
    int flag = fcntl(client->sock, F_GETFL, 0);       // 1.1 it returns (flag) a bitwise mask of current options
    fcntl(client->sock, F_SETFL, flag | O_NONBLOCK);  // 2.1 sets the socket to Non- blocking  mode  

    while (clinet_active) {

        // Receive a reply (unchanged, but ensure null-termination)
        char buffer_recv[4096];
        ssize_t recv_size = recv(client->sock, buffer_recv, sizeof(buffer_recv) - 1, 0);
        if (recv_size > 0)
            {
            buffer_recv[recv_size] = '\0';

            if (debug)
                {
                display_msg_safe(FG_CYAN, buffer_recv, recv_size);   //in future replace the argument 1 with other_client color
                pthread_mutex_lock(&display_lck);
                printf("[DEBUG MODE]Recieved :%zd bytes\n", recv_size);
                rl_forced_update_display();
                pthread_mutex_unlock(&display_lck);
                }
            else
                {
                display_msg_safe(client->cli_display_color, buffer_recv, recv_size);   //in future replace the argument 1 with other_client color
                }
            }
        else if (recv_size == 0) {
            pthread_mutex_lock(&display_lck);
            printf("\n%sServer closed the connection.%s\n", FG_BRED, RESET);
            rl_forced_update_display();
            pthread_mutex_unlock(&display_lck);
            break;
            }
        else if (recv_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            usleep(10000);  // 10ms
            }
        else {
            pthread_mutex_lock(&display_lck);
            perror("recv");
            rl_forced_update_display();
            pthread_mutex_unlock(&display_lck);
            break;
            }
        }
    clinet_active = false;
    return NULL;
    }

// ---------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
    {

    // CLA checking 
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
    // socket establishment process
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
    if (!client_info_t) {
        fprintf(stderr, "[%s Error %s] | Memory allocation failed\n", FG_RED, RESET);
        close(sock);
        return 1;
        }

    char buffer[META_D_BUFFER_SIZE];
    ssize_t n_byte;

    printf("%s%sChoose a name to connect server :%s\t", UNDERLINE, FG_BBLUE, RESET);
    fflush(stdout);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
        fprintf(stderr, "[%s Error %s] | Failed to read name\n", FG_RED, RESET);
        free(client_info_t);
        close(sock);
        return 1;
        }

    client_info_t->client_name = strdup(buffer);
    client_info_t->sock = sock;

    // Fetch or generate the uuid of client 
    client_info_t->cli_uuid = (char*)malloc(37 * sizeof(char));
    uuid_fetch(client_info_t->cli_uuid, debug);
    combine_msg(buffer, client_info_t->cli_uuid, debug);
    if (!client_info_t->cli_uuid) {
        fprintf(stderr, "[%s Error %s] | Memory allocation failed\n", FG_RED, RESET);
        free_client(client_info_t);
        close(sock);
        return 1;
        }

    // send meta data to the server 
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("send Meta Data :");
        free_client(client_info_t);
        return 1;
        }
    // recieve server info || 
    if ((n_byte = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[n_byte] = '\0';

        if (!break_meta_d(&client_info_t, buffer)) {
            fprintf(stderr, "[%s Error %s] | Failed to parse server metadata\n", FG_RED, RESET);
            free_client(client_info_t);
            close(sock);
            return 1;
            }
        if (debug) {
            printf("Recieved :%s\n", client_info_t->server_name);
            printf("Recieved :%sxxxxx%s\n", client_info_t->cli_display_color, RESET);
            }

        }
    else {
        perror("Recv Meta Data :");
        free_client(client_info_t);
        close(sock);
        return 1;
        }

    printf("Connected to server [ IP= %s%s%s ] [ Port= %s%d%s ]\n", FG_GREEN, server_ip, RESET, FG_GREEN, port, RESET);
    if (debug)
        {
        // Meta data of client
        printf("%s%s\n\t\t Meta Data Of Client Sended to the server%s\n\n", FG_BBLUE, BOLD, RESET);
        printf("Server Name : %s\n", client_info_t->server_name);
        printf("Client Name : %s\n", client_info_t->client_name);
        printf("clinet uuid : %s\n", client_info_t->cli_uuid);
        // printf("MAC Address: %s\n");
        }

    // Seperate thread to recieve the data /msg
    pthread_t recever_th;
    if (pthread_create(&recever_th, NULL, recever_thread, client_info_t)) {
        fprintf(stderr, "[%s Error %s] | Failed to create receiver thread\n", FG_RED, RESET);
        free_client(client_info_t);
        close(sock);
        return 1;
        }
    puts(" ");

    // set up readline
    rl_catch_signals = 0;

    char prompt[128];
    snprintf(prompt, sizeof(prompt), "%s>>>%s ", FG_BGREEN, RESET);

    printf("%s", prompt);
    fflush(stdout);
    // msg sending loop
    while (clinet_active) {

        // select to check if stdin has data . making readline non-blocking 
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

        if (ready < 0) {
            if (errno == EINTR) {
                continue;
                }
            fprintf(stderr, "[%s Error %s] | ", FG_RED, RESET);
            perror("Select:");
            break;
            }
        // when no input ,just continue loop
        if (ready == 0) {
            continue;
            }
        
        // when input avaliable
        pthread_mutex_lock(&display_lck);
        char* line = readline(prompt);
        pthread_mutex_unlock(&display_lck);

        //  condition of : End od file (Crtl +D) 
        if (line == NULL) {
            printf("\nEOF detected.Exiting.\n");
            break;
            }

        // Skip empty line
        if (strlen(line) == 0) {
            free(line);
            continue;
            }

        //add the line to history
        add_history(line);

        // check quit command  
        if (quit_check(line)) {
            puts("Exiting...");
            free(line);
            break;
            }

        // add \n for transmission
        size_t line_len = strlen(line);
        char* line_wt_newline = (char*)malloc(line_len + 2);
        if (!line_wt_newline) {
            fprintf(stderr, "[%s Error %s] | Memory allocation failed for line_wt_newline\n", FG_RED, RESET);
            free(line);
            break;
            }
        strcpy(line_wt_newline, line);
        line_wt_newline[line_len] = '\n';
        line_wt_newline[line_len + 1] = '\0';

        // send the user input
        if (send_all(sock, line_wt_newline, (size_t)line_len + 1) < 0) {
            fprintf(stderr, "[%s Error %s] | send_all\n", FG_RED, RESET);
            free(line_wt_newline);
            free(line);
            break;
            }
        if (debug) {
            pthread_mutex_lock(&display_lck);
            printf("[DEBUG] Sent: %zu bytes\n", line_len + 1);
            pthread_mutex_unlock(&display_lck);
            }

        free(line_wt_newline);
        free(line);

        }
    // cleanup 
    clinet_active = false;
    printf("\nDisconnecting...\n");
    if (debug) {
        printf("\nWaiting for receiver thread to finish...\n");
        }
    pthread_join(recever_th, NULL);
    free_client(client_info_t);
    close(sock);

    rl_clear_history();
    if (debug) {
        printf("client shutdown complete. \n");
        }
    return 0;
    }