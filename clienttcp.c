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

// GLOBAL TIMESTAMP
long first_timestamp_ms = -1;

// ---- TCP INFO LOGGING ----
void log_tcp_info(FILE *logfile, const char *source, int sockfd) {
    struct tcp_info info;
    socklen_t len = sizeof(info);
    struct timeval tv;

    gettimeofday(&tv, NULL);
    long current_ms = tv.tv_sec * 1000L + tv.tv_usec / 1000;

    if (first_timestamp_ms < 0) {
        first_timestamp_ms = current_ms;
    }

    long timestamp_ms = current_ms - first_timestamp_ms;

    if (getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, &len) == 0) {
        fprintf(logfile,
            "%s,%ld,%u,%u,%u,%u,%u,%u\n",
            source,
            timestamp_ms,
            info.tcpi_rtt,
            info.tcpi_rttvar,
            info.tcpi_snd_cwnd,
            info.tcpi_snd_ssthresh,
            info.tcpi_retrans,
            info.tcpi_lost
        );
        fflush(logfile);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Usage: %s <remote_host> <remote_port> <msg_size> <num_msgs>\n", argv[0]);
        exit(1);
    }

    char *rem_hostname = argv[1];
    int rem_port = atoi(argv[2]);
    int msg_size = atoi(argv[3]);
    long num_msgs = atol(argv[4]);

    struct sockaddr_in rem_addr;
    int rem_sockfd;

    // ---- OPEN CSV LOG ----
    FILE *logfile = fopen("client_metrics.csv", "w");
    if (!logfile) {
        perror("fopen");
        exit(1);
    }

    fprintf(logfile,
        "source,timestamp_ms,rtt,rttvar,cwnd,ssthresh,retrans,lost\n"
    );
    fflush(logfile);

    // Allocate send buffer
    char *buffer = malloc(msg_size);
    if (!buffer) {
        perror("malloc");
        exit(1);
    }
    memset(buffer, 'A', msg_size);

    // Create TCP socket
    rem_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rem_sockfd < 0) {
        perror("socket");
        exit(1);
    }

    rem_addr.sin_family = AF_INET;
    rem_addr.sin_addr.s_addr = inet_addr(rem_hostname);
    rem_addr.sin_port = htons(rem_port);

    printf("> Connecting to server %s:%d\n", rem_hostname, rem_port);

    if (connect(rem_sockfd, (struct sockaddr *)&rem_addr, sizeof(rem_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("> Sending %ld messages of %d bytes...\n", num_msgs, msg_size);

    for (long i = 0; i < num_msgs; i++) {
        if (send(rem_sockfd, buffer, msg_size, 0) < 0) {
            perror("send");
            break;
        }

        // Log TCP state after each send
        log_tcp_info(logfile, "client_to_server", rem_sockfd);
    }

    printf("> Done.\n");

    fclose(logfile);
    close(rem_sockfd);
    free(buffer);

    return 0;
}

