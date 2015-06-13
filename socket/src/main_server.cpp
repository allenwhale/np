#include "server.h"
#include <stdio.h>
int main(){
    Server server = Server(6000, 0, 4);
    if(server.Connect() < 0){
        return 0;
    }
    server.Run();
    server.Close();
    return 0;
}
