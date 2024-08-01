#include "net_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BACKLOG 10 // max incoming connection queue

/*
 * Figures out whether its an IPv4 or IPv6 address.
 */
void *inaddr_type(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/*
 * Returns listener socket
 */
int conn_listener(char *port) {
  int sockfd;
  struct addrinfo hints, *res, *res_ptr;
  int bool = 1;
  int getaddrval; // getaddrinfo() returns 0 on success, any other int on error

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     // either IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP (stream)
  hints.ai_flags = AI_PASSIVE;     // uses current IP

  /* on success, (*res) will be a pointer to a linked list containing multiple
     addrinfo structs */
  if ((getaddrval = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrval));
    return -1;
  }

  printf("[Odis] Trying to establish a socket connection (...)\n");
  for (res_ptr = res; res_ptr != NULL; res_ptr = res_ptr->ai_next) {
    // attempt to create a socket based on the current interface
    if ((sockfd = socket(res_ptr->ai_family, res_ptr->ai_socktype,
                         res_ptr->ai_protocol)) == -1) {
      continue;
    }

    // avoid "address already in use" errors
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &bool, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      close(sockfd);
      freeaddrinfo(res); // all done with this structure
      return -2;
    }

    // attempt to bind the ip address to the current socket
    if (bind(sockfd, res_ptr->ai_addr, res_ptr->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }

    // yeeey, we managed to bind to a socket!!!!
    break;
  }

  freeaddrinfo(res);

  /*
   * in case we didnt break out of the loop it means we didnt manage to bind to
   * any socket, therefore *res_ptr will be NULL (after iteration over linked
   * list above)
   */
  if (res_ptr == NULL) {
    fprintf(stderr, "[Odis] Failed to find local address.\n");
    return -3;
  }

  // else, we managed to bind and now we're listening on that IP
  if (listen(sockfd, BACKLOG) == -1) {
    close(sockfd);
    return -4;
  }

  printf("[Odis] Succesful socket connection -> 127.0.0.1:%s\n", port);
  printf("[Odis] Listening (...)\n");

  return sockfd;
}
