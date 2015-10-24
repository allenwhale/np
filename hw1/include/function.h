#define FSTDOUT fflush(stdout)
#define FSTDERR fflush(stderr)
#define BUFFER_SIZE 262144
#define MAX_EPOLL_SIZE 1024
#include <bits/stdc++.h>
void epoll_add_event(int, int, int);
void epoll_del_event(int, int, int);
int string2int(const std::string&);
