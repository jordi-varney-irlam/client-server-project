#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in address;
    int sock_fd;
    char buf[1024];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 aka localhost
    address.sin_port = htons(4124);
    // ^ arbitrary port number

    if (-1 == connect(sock_fd, (struct sockaddr *)&address, sizeof(address))) {
        perror("connect");
        return 1;
    }

    // make stdin nonblocking
    if (-1 == fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
	    perror("fcntl stdin NONBLOCK");
	    close(sock_fd);
	    return 1;
    }

    // make the socket nonblocking
    if (-1 == fcntl(sock_fd, F_SETFL, O_NONBLOCK)) {
	    perror("fcntl sock_fd NONBLOCK");
	    close(sock_fd);
	    return 1;
    }

    FILE *server = fdopen(sock_fd, "r+");

    while (1) {
        // tries  to read from stdin, and forward across the socket
        // tries to read from the socket, and forward to stdout
	errno = 0;
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            if (fprintf(server, "%s", buf) < 0) {
                fprintf(stderr, "Failed to send message to server.\n");
                break;
            }
	    fflush(server);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("fgets");
            break;
        }

       
        errno = 0;
        if (fgets(buf, sizeof(buf), server) != NULL) {
            printf("%s", buf);
            fflush(stdout);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "Server error occurred.\n");
            break;
        }

        usleep(100 * 1000); // wait 100ms before checking again
    }
    fclose(server);
    return 0;
    }
