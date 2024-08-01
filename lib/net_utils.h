#include <sys/socket.h>
#ifndef _NET_UTILS_H_
#define _NET_UTILS_H_

void *get_in_addr(struct sockaddr *sa);
int conn_listener(char *port);

#endif
