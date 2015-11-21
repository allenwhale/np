#include "function.h"
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/file.h>
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

int lock_open(const char *file, int op){
    int fd = open(file, op);
    flock(fd, LOCK_EX);
    return fd;
}

int unlock_close(int fd){
    flock(fd,  LOCK_UN);
    return close(fd);
}

void set_nonblock(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}
