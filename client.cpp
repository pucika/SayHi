#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAXSIZE 1024
#define IPADDRESS "127.0.0.1"
#define SERV_PORT 1111
#define FDSIZE 1024
#define EPOLLEVENTS 20

void handle_connection(int sockfd);
void handle_events(int epollfd, struct epoll_event *events, 
					int num, int sockfd, char *buf);
void do_read(int epollfd, int fd, int sockfd, char* buf);
void do_write(int epollfd, int fd, int sockfd, char* buf);
void add_event(int epollfd, int fd, int state);
void delete_event(int epollfd, int fd, int state);
void modify_event(int epollfd, int fd, int state);

int main(int argc, char* argv[]) {
	int sockfd;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
	connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	handle_connection(sockfd);
	close(sockfd);
	return 0;
}

void handle_connection(int sockfd) {
	int epollfd;
	struct epoll_event events[EPOLLEVENTS];
	char buf[MAXSIZE];
	memset(buf, 0, MAXSIZE);
	int ret;
	epollfd = epoll_create(FDSIZE);
	add_event(epollfd, STDIN_FILENO, EPOLLIN);
	add_event(epollfd, sockfd, EPOLLIN);
	//add_event(epollfd, STDOUT_FILENO, EPOLLOUT);
	while(1) {
		ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
		handle_events(epollfd, events, ret, sockfd, buf);
	}
}

void handle_events(int epollfd, struct epoll_event *events, 
	int num, int sockfd, char *buf) {
	int fd;
	for(int i = 0; i != num; ++i) {
		fd = events[i].data.fd;
		if (events[i].events & EPOLLIN) 
			do_read(epollfd, fd, sockfd, buf);
		else if (events[i].events & EPOLLOUT)
			do_write(epollfd, fd, sockfd, buf);
	}
}

void do_read(int epollfd, int fd, int sockfd, char *buf) {
	int nread;
	memset(buf, 0, MAXSIZE);
	nread = read(fd, buf, MAXSIZE);
	if (nread == -1) {
		perror("read error\n");
		close(fd);
	} else if (nread == 0) {
		fprintf(stderr, "peer close.\n");
		close(fd);
		exit(0);
	} else {
		if (fd == STDIN_FILENO) {
			write(sockfd, buf, MAXSIZE);
			modify_event(epollfd, sockfd, EPOLLIN);
		}
		else if (fd == sockfd) {
			write(STDOUT_FILENO, buf, MAXSIZE);
			//modify_event();
		}
	}
}

void do_write(int epollfd, int fd, int sockfd, char *buf) {
	int nwrite = write(fd, buf, MAXSIZE);
	if (nwrite == -1) {
		perror("write error\n");
		close(fd);
	} else if (fd != STDOUT_FILENO)
		modify_event(epollfd, fd, EPOLLIN);
	memset(buf, 0, MAXSIZE);
}

void add_event(int epollfd, int fd, int state) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void modify_event(int epollfd, int fd, int state) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void delete_event(int epollfd, int fd, int state) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}