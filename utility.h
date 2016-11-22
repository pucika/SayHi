#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include "redis.h"

#define EPOLLEVENTS 100
#define MAXSIZE 512
#define LISTENQ 100
#define FDSIZE 1000
extern struct epoll_event events[EPOLLEVENTS];

int socket_bind(const char* ip, int port);
void do_epoll(int listenfd);
void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf);
void handle_accept(int epollfd, int listenfd);
void do_read(int epollfd, int fd, char *buf);
void do_write(int epollfd, int fd, char* buf);
void add_event(int epollfd, int fd, int state);
void modify_event(int epollfd, int fd, int state);
void delete_event(int epollfd, int fd, int state);
void add_user(const char* ip);       //ip vs port
void del_user(const char* ip);
int to_number(std::string str);

#endif