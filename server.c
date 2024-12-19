#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CLIENTS 4
FILE *CLIENTS[MAX_CLIENTS] = {0};
// ^ Each connected client is represented as a FILE*
// If that client is not connected, its FILE* will be NULL

/** Sends the message `buf` to all connected clients except
 * the one at `sender_index`.
 * */
void redistribute_message(int sender_index, char *buf) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
 	    if (i == sender_index || CLIENTS[i] == NULL) continue;

            if (fprintf(CLIENTS[i], "%s", buf) < 0 || fflush(CLIENTS[i]) == EOF) {
            fclose(CLIENTS[i]);
            CLIENTS[i] = NULL;
            fprintf(stderr, "Message sending failure\n");
        }
    }
}

/** Tries to read a message from the specified client.:
 * If any other kind of error occurred, close the client and reset its FILE* to NULL.
 * Return a boolean indicating whether a message was actually read from the client.
 */
int poll_message(char *buf, size_t len, int client_index) {
	if (CLIENTS[client_index] == NULL){
	       	return 0;
	}

        errno = 0;
        if (fgets(buf, len, CLIENTS[client_index]) == NULL) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            } else {
                fclose(CLIENTS[client_index]);
                CLIENTS[client_index] = NULL;
                fprintf(stderr, "Polling message error\n");
            }
        } else {
            return 1;
        }
        return 0;
}

void try_add_client(int server_fd) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            } else {
                perror("accept");
                exit(1);
            }
        }

        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
            perror("fcntl client_fd NONBLOCK");
            close(client_fd);
            return;
        }

        FILE *client_file = fdopen(client_fd, "r+");
        if (client_file == NULL) {
            perror("fdopen");
            close(client_fd);
            return;
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (CLIENTS[i] == NULL) {
                CLIENTS[i] = client_file;
                fprintf(client_file, "Hello chat server!\n");
                fflush(client_file);
                return;
            }
        }

	const char *full_message = "Room is - unfortunately - full.\n";
	fprintf(client_file, "%s", full_message);
	fflush(client_file);
	close(client_fd);

}

int main_loop(int server_fd)
{
    char buf[1024];

    while (1) {
        // check each client to see if there are messages to redistribute
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (NULL == CLIENTS[i]) continue;
            if (!poll_message(buf, sizeof(buf), i)) continue;
            printf("Received message from client %i: %s", i, buf);
            redistribute_message(i, buf);
        }

        // see if there's a new client to add
        try_add_client(server_fd);

        usleep(100 * 1000); // wait 100ms before checking again
    }
}

int main(int argc, char* argv[])
{
    struct sockaddr_in address;
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 aka localhost
    // only listen for incoming connections that come from 127.0.0.1 i.e. the same
    // computer that the server process is running on.
    address.sin_port = htons(4124);

    if (-1 == bind(server_fd, (struct sockaddr *)&address, sizeof(address))) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (-1 == listen(server_fd, 5)) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    // set the server file descriptor to nonblocking mode
    // so that `accept` returns immediately if there are no pending
    // incoming connections
    if (-1 == fcntl(server_fd, F_SETFL, O_NONBLOCK)) {
        perror("fcntl server_fd NONBLOCK");
        close(server_fd);
        return 1;
    }

    int status = main_loop(server_fd);
    close(server_fd);
    return status;
}
