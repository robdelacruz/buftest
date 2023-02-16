#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "sockbuf.h"

// Print the last error message corresponding to errno.
void print_err(char *s) {
    fprintf(stderr, "%s: %s\n", s, strerror(errno));
}
void exit_err(char *s) {
    print_err(s);
    exit(1);
}

int main(int argc, char *argv[]) {
    int z;
    int servsock;
    char buf[1024];

    if (argc < 3) {
        printf("Usage: tclient <server domain> <port>\n");
        printf("Ex. tclient 127.0.0.1 5000\n");
        exit(1);
    }

    char *server_domain = argv[1];
    char *server_port = argv[2];
    struct addrinfo hints, *servaddr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    z = getaddrinfo(server_domain, server_port, &hints, &servaddr);
    if (z != 0) {
        exit_err("getaddrinfo()");
    }

    // Server socket
    servsock = socket(servaddr->ai_family, servaddr->ai_socktype, servaddr->ai_protocol);
    if (servsock == -1) {
        exit_err("socket()");
    }

    z = connect(servsock, servaddr->ai_addr, servaddr->ai_addrlen);
    if (z != 0) {
        exit_err("connect()");
    }

    freeaddrinfo(servaddr);
    servaddr = NULL;

    printf("Connected to %s:%s\n", server_domain, server_port);

    sockbuf_t *servsb = sockbuf_new(servsock, 0);

    char *msg = "Now is the time for all good men to come to the aid of the party.\n";
    char *msg2 = "The quick brown fox jumps over the lazy dog.\n";
    send(servsock, msg, strlen(msg), 0);
    send(servsock, msg2, strlen(msg2), 0);

    sleep(1);

    int servmsglen = sockbuf_recv(servsb, buf, sizeof buf);
    if (servmsglen == 0) {
        printf("recv(): EOF\n");
    }
    buf[servmsglen] = '\0';
    printf("recv(): %s\n", buf);

    close(servsock);
    return 0;
}
