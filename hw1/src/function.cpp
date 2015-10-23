#include "function.h"
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>

int non_blocking(int fd){
	int flag;
	if((flag = fcntl(fd, F_GETFL, 0)) == -1)
		return -1;
	if(fcntl(fd, F_SETFL, flag|O_NONBLOCK) == -1)
		return -1;
	return 0;
}

void epoll_add_event(int epfd, int fd, int state){
	epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

void epoll_del_event(int epfd, int fd, int state){
	epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
}
