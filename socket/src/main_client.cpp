#include "client.h"
#include <stdio.h>
int main(){
    Client client("127.0.0.1", 6000, 0);
    if(client.Connect() < 0){
        return 0;
    }
    puts("end");
    client.Run();
    client.Close();
    return 0;
}
