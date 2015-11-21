#include "client.h"
#include <string.h>

Client::Client(): used(false){
    memcpy(nickname, "(no name)\0", sizeof(char) * 10);
}

void Client::set_nickname(const char *nick){
    memset(nickname, 0, sizeof(nickname));
    memcpy(nickname, nick, sizeof(char) * strlen(nick));
}
