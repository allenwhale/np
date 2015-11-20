#include <bits/stdc++.h>
#include "client.h"

Client::Client(): used(false) {}
Client::Client(int _id, int _fd, std::string _ip, int _port): used(true), id(_id), fd(_fd), nickname("(no name)"), ip(_ip), port(_port){}
