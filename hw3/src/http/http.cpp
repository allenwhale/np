#include "http.h"
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
#include "server.h"
#include "http.h"
using namespace std;

int main(){
    Server server = Server(PORT, 10);
    server.SetCGI({".cgi"});
    if(server.Connect() < 0)
        exit(1);
    server.Run();
    return 0;
}
