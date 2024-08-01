/*
 * Odis - A minimal webserver written in C
 *
 * Can be tested via cURL:
 *    curl -D - http://localhost:PORT/
 *    curl -D - http://localhost:PORT/d20
 *
 * Or directly in your browser, just access the URLs above!
 *
 * POST request:
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!'
 * http://localhost:PORT/save
 *
 * (Posting data is harder to test from a browser, I recommend Postman or
 * Insomnia)
 */

#include "../lib/net_utils.h"
#include <fcntl.h> // for open
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 2048

// Function to serve a file
void serve_file(int clientfd, const char *filename, const char *content_type) {
  int filefd = open(filename, O_RDONLY);
  if (filefd == -1) {
    perror("open");
    char *not_found_response = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 13\r\n"
                               "\r\n"
                               "404 Not Found";
    send(clientfd, not_found_response, strlen(not_found_response), 0);
    return;
  }

  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  char headers[256];
  snprintf(headers, sizeof(headers),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           content_type);

  send(clientfd, headers, strlen(headers), 0);

  while ((bytes_read = read(filefd, buffer, sizeof(buffer))) > 0) {
    send(clientfd, buffer, bytes_read, 0);
  }

  close(filefd);
}

char *read_file(const char *filename, size_t *filesize) {
  printf("[Odis] Serving files: \n\t Entry point: %s\n", filename);
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("fopen");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(length);
  if (buffer == NULL) {
    perror("malloc");
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, length, file);
  fclose(file);

  *filesize = length;
  return buffer;
}

int main(int argc, char *argv[]) {
  int clientfd;
  struct sockaddr client;
  socklen_t client_addrsize;
  if (argc != 3) {
    fprintf(stderr, "[Odis] Usage: ./server <port>\n");
    printf("%s %s", argv[1], argv[2]);
    exit(1);
  }

  printf(
      "\n---------------------------------------------------------------------"
      "-----"
      "--------------\n");
  printf("| Odis | A minimalist, dependency-free webserver written in pure "
         "C (HTTP 1.1/RFC 2616) |\n");
  printf(
      "----------------------------------------------------------------------"
      "----"
      "----"
      "----------\n\n");
  if (strcmp(argv[1], "-p") == 0) {
    int sockfd_listener = conn_listener(argv[2]);

    while (1) {
      client_addrsize = sizeof client;
      clientfd =
          accept(sockfd_listener, (struct sockaddr *)&client, &client_addrsize);
      if (clientfd == -1) {
        perror("[Odis] Server accept error\n");
        continue;
      }
      printf("[Odis] Someone visited 127.0.0.1:%s\n", argv[2]);

      char response_headers[256];

      // Read the request
      char request[BUFFER_SIZE];
      int bytes_received = recv(clientfd, request, sizeof(request) - 1, 0);
      if (bytes_received < 1) {
        close(clientfd);
        continue;
      }
      request[bytes_received] = '\0';

      // Parse the request line
      char method[16], path[256], protocol[16];
      sscanf(request, "%s %s %s", method, path, protocol);

      // Determine file path and content type
      char filepath[512];
      const char *content_type = "text/html";
      if (strcmp(path, "/") == 0) {
        snprintf(filepath, sizeof(filepath), "./static_content/index.html");
      } else {
        snprintf(filepath, sizeof(filepath), "./static_content%s.html", path);
      }

      // Serve the requested file
      serve_file(clientfd, filepath, content_type);

      // Serve the requested file
      // if (strcmp(path, "/") == 0) {
      //   // Serve the main HTML file
      //   serve_file(clientfd, "./static_content/index.html", "text/html");
      // }else if (strcmp(path, "/about")) {
      //   serve_file(clientfd, "./static_content/.html", "text/html");
      //
      // } else {
      //   // Serve 404 Not Found for other requests
      //   char *not_found_response = "HTTP/1.1 404 Not Found\r\n"
      //                              "Content-Type: text/plain\r\n"
      //                              "Content-Length: 13\r\n"
      //                              "\r\n"
      //                              "404 Not Found";
      //   send(clientfd, not_found_response, strlen(not_found_response), 0);
      // }

      close(clientfd); // Close the connection to the client
    }

    close(sockfd_listener); // Close the listening socket
  }
  return 0;
}
