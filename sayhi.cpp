#include "utility.h"

extern Redis* redis = new Redis();

int main(int argc, char* argv[])
{
    int listened;
    listenfd = socket_bind(INADDR_ANY, 1111);
    listen(listenfd, 50);
    do_epoll(listenfd);
    return 0;
}


