#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>

//VARIAVEIS GLOBIAS
long first_timestamp_ms = -1; //inicia a primeira medição
//FUNÇÕES AUXILIARES
void log_tcp_info(FILE *logfile, const char *source, int sockfd);

int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Usage: %s <proxy_port> <server_ip> <server_port> <msg_size>\n", argv[0]);
        exit(1);
    }

    int proxy_port = atoi(argv[1]);
    char *server_ip = argv[2];
    int server_port = atoi(argv[3]);
    int msg_size   = atoi(argv[4]);

    int proxy_sockfd, client_sockfd, server_sockfd;
    struct sockaddr_in proxy_addr, client_addr, server_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    // --- OPEN LOG FILE ---
    FILE *logfile = fopen("metrics.csv", "w");
    if (!logfile) {
        perror("fopen");
        exit(1);
    }

    // CSV header
    fprintf(logfile,
        "source,timestamp_ms,rtt,rttvar,cwnd,ssthresh,retrans,lost\n"
    );
    fflush(logfile);

    // Allocate buffer
    char *buffer = malloc(msg_size);
    if (!buffer) { perror("malloc"); exit(1); }

    // --- Create proxy listener ---
    proxy_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_sockfd < 0) { perror("socket"); exit(1); }

    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(proxy_port);
    bzero(&(proxy_addr.sin_zero), 8);

    if (bind(proxy_sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(proxy_sockfd, 1);
    printf("> Proxy listening on port %d\n", proxy_port);

    // Accept client
    client_sockfd = accept(proxy_sockfd, (struct sockaddr *)&client_addr, &addrlen);
    printf("> Client connected\n");

    // --- Connect to server ---
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) { perror("socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (connect(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }
    printf("> Connected to server %s:%d\n", server_ip, server_port);

    long long total_from_client = 0;
    long long total_to_server  = 0;

    // --- Forward Loop ---
    while (1) {
        int received = recv(client_sockfd, buffer, msg_size, 0);
        if (received <= 0) break;

        
        total_from_client += received;

        // forward to server
        int sent = send(server_sockfd, buffer, received, 0);
        total_to_server += sent;

        // Print debug
        //printf("Forwarded %d bytes\n", received);

        // --- LOGGING ---
        log_tcp_info(logfile, "client_to_proxy", client_sockfd);
        log_tcp_info(logfile, "proxy_to_server", server_sockfd);
    }

    printf("> Client disconnected. Closing proxy.\n");
    printf("Total bytes received from client: %lld\n", total_from_client);
    printf("Total bytes sent to server: %lld\n", total_to_server);

    fclose(logfile);
    close(client_sockfd);
    close(server_sockfd);
    close(proxy_sockfd);
    free(buffer);
    return 0;
}


//coleta informações da conexão
void log_tcp_info(FILE *logfile, const char *source, int sockfd) {
    struct tcp_info info;
    socklen_t len = sizeof(info);
    struct timeval tv;

    // Get current timestamp
    gettimeofday(&tv, NULL);
    long current_ms = tv.tv_sec * 1000L + tv.tv_usec / 1000;

    // Initialize reference timestamp on first measurement
    if (first_timestamp_ms < 0) {
        first_timestamp_ms = current_ms;
    }

    long timestamp_ms = current_ms - first_timestamp_ms;

    if (getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, &len) == 0) {
        fprintf(logfile,
            "%s,%ld,%u,%u,%u,%u,%u,%u\n",
            source,
            timestamp_ms,           // timestamp (ms)
            info.tcpi_rtt,          // RTT
            info.tcpi_rttvar,       // RTT variation
            info.tcpi_snd_cwnd,     // cwnd
            info.tcpi_snd_ssthresh, // ssthresh
            info.tcpi_retrans,      // retransmissions
            info.tcpi_lost          // lost packets
        );
        fflush(logfile);
    }
}


