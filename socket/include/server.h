#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <unordered_map>
#define BUFFER_SIZE 2048

class Server{
private:
    int                 sockfd;
    sockaddr_in         dest;
    int                 port;
    int                 timeout;
    timeval             tv;
    int                 connection;
    std::vector<int>    clifd;
    std::unordered_map<int, std::string> clinick;
    fd_set              infdset;
public:
    Server(int, int, int);
    //port, timeout, connection
    void SetCliFd(int);
    void SetFdNick(int, const char *);
    void SetFd();
    int Connect();
    void Run();
    int ServerProcess();
    int StdinProcess();
    int ClientProcess();
    int SendAll(const char *, int, const std::vector<int>&);
    void Close();
};
