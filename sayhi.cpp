#include <sys/types.h>
#include "utility.h"

extern Redis* redis;
struct epoll_event events[EPOLLEVENTS];
int main(int argc, char* argv[])
{
	if(!redis->connect("127.0.0.1", 6379))
	{
		printf("connect error!\n");
		return 0;
	}
    int listenfd;
    listenfd = socket_bind(INADDR_ANY, 1111);
    if (listen(listenfd, 50) == -1) {
    	perror("listen error");
    	return -1;
    }
    do_epoll(listenfd);
    return 0;
}


