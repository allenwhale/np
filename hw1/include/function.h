#define FSTDOUT fflush(stdout)
#define FSTDERR fflush(stderr)
#define BUFFER_SIZE 2048
#define MAX_EPOLL_SIZE 1024
int non_blocking(int);
void epoll_add_event(int epfd, int fd, int state);
void epoll_del_event(int epfd, int fd, int state);
