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
	return std::stoi(str);
}

bool isdigit(const std::string &str){
    return std::all_of(str.begin(), str.end(), [](char s){return isdigit(s);});
}
