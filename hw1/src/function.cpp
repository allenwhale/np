#include "function.h"
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>

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

int string2int(const std::string& str){
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}
