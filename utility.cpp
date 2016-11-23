#include "utility.h"


#define IP "127.0.0.1"

Redis* redis = new Redis();

int socket_bind(const char* ip, int port) {
	int listenfd;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listenfd) {
		perror("socket error");
		exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind error");
		exit(1);
	}
	printf("bind OK!\n");
	return listenfd;
}

void do_epoll(int listenfd) {
	printf("do_epoll\n");
	int epollfd;
	int ret;
	char buf[MAXSIZE];
	memset(buf, 0, MAXSIZE);

	epollfd = epoll_create(FDSIZE);

	add_event(epollfd, listenfd, EPOLLIN);

	while(1) {
		ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
		handle_events(epollfd, events, ret, listenfd, buf);
	}
}

void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf) {
	int fd;
	for(int i = 0; i < num; ++i) {
		fd = events[i].data.fd;
		if (fd == listenfd && (events[i].events & EPOLLIN))
			handle_accept(epollfd, listenfd);
		else if (events[i].events & EPOLLIN)
			do_read(epollfd, fd, buf);					//TODO 分发队列
		else if (events[i].events & EPOLLOUT)
			do_write(epollfd, fd, buf);					//TODO 分发队列
	}
}

void handle_accept(int epollfd, int listenfd){
	int chifd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen;
	chifd = accept(listenfd, (struct sockaddr*) &cliaddr, &cliaddrlen);
	if (chifd == -1)
		perror("accept error");
	else {
		//判断ip是否在用户表单	
		//TODO
		//redis->sadd("user", "");
		// 如果没有，添加ip至用户表单	以及匹配信息	：user(set)  mapping(ht)ip:fd
		if (redis->scard("user") == 0)	{
			char buf[13];
			memset(buf, 0, sizeof(buf));
			snprintf(buf, 13, "matching...\n");
			//buf[12] = '\0';
			write(chifd, buf, strlen(buf));
			redis->sadd("user", std::to_string(chifd));
		} else {
			int candi = redis->spopi("user");
			//printf("candi: %d", candi);
			//printf("chifd: %d", chifd);
			redis->hset("match", std::to_string(candi), std::to_string(chifd));
			redis->hset("match", std::to_string(chifd), std::to_string(candi));
			add_event(epollfd, chifd, EPOLLIN);
			add_event(epollfd, candi, EPOLLIN);
			char buf[50];
			memset(buf, 0, sizeof(buf));
			snprintf(buf, 50, "match complete! you can send some words.\n");
			//strncpy(buf, temp, MAXSIZE);
			write(chifd, buf, strlen(buf));
			write(candi, buf, strlen(buf));
		}
		//get random mapping 若没有，返回；有，匹配   match(ht): fd:fd

	}
}

void do_read(int epollfd, int fd, char *buf) {
	int nread;
	nread = read(fd, buf, MAXSIZE);
	if (nread == -1) {
		perror("read error: ");
		int fd2 = redis->hget("match", std::to_string(fd));
		//char buf[] = "peer closed!";
		//write(fd2, buf, sizeof(buf));
		if (fd2 == 0)
			redis->srem("user", std::to_string(fd));
		else {
			redis->hdel("match", std::to_string(fd), std::to_string(fd2));
			close(fd);
			close(fd2);
		}
	} else if (nread == 0) {
		//关闭描述符
		int fd2 = redis->hget("match", std::to_string(fd));
		//char buf[] = "peer closed!";
		//write(fd2, buf, sizeof(buf));
		if (fd2 == 0)
			redis->srem("user", std::to_string(fd));
		else {
			redis->hdel("match", std::to_string(fd), std::to_string(fd2));
			close(fd);
			close(fd2);
		}

	} else {
		// got matched fd2
		int fd2 = redis->hget("match", std::to_string(fd));
		write(fd2, buf, strlen(buf) + 1);
		memset(buf, 0, MAXSIZE);
		//do_write(epollfd, fd2, buf);
		modify_event(epollfd, fd, EPOLLIN);
		modify_event(epollfd, fd2, EPOLLIN);
	}
}

void do_write(int epollfd, int fd, char* buf) {
	int nwrite;
	nwrite = write(fd, buf, strlen(buf) + 1);
	if (nwrite == -1) {
		perror("write error: ");
		close(fd);
		delete_event(epollfd, fd, EPOLLOUT);
	} else{
		//int fd2 = redis->hget("match", std::to_string(fd));
		//write(fd2, buf, sizeof(buf));
		modify_event(epollfd, fd, EPOLLIN);
	}
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

void add_user(const char* ip) {	//ip vs port
	redis->sadd("user", ip);		//TODO设置过期时间
}   

void del_user(const char* ip) {
	redis->srem("user", ip);
}

int to_number(std::string str)
{
	std::stringstream ss;
	ss << str;
	int ret ;
	ss >> ret;
	return ret;
}