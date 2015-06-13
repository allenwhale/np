#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#define IP_SIZE 32
class Client{
private:
    int         sockfd;
    sockaddr_in dest;
    int         port;
    int         timeout;
    char        ip[IP_SIZE];
    fd_set      infdset;
    timeval     tv;

public:
    Client(const char *, int, int);
    int Connect();
    void SetFd();
    void Run();
    int StdinProcess();
    int ServerProcess();
    void Close();
};
