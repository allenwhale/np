#include <bits/stdc++.h>
#include <unistd.h>
#include "hw3.h"
#include "client.h"
using namespace std;

int main(){
    header();
    args_map args = parse_args(getenv("QUERY_STRING"));
    body(args);
    fflush(stdout);
    run(args);
    return 0;
}

void header(){
    printf("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
}

string urldecode(const string &str){
    string res;
    unsigned int tmp;
    for(int i=0;i<(int)str.length();i++){
        if(str[i] == 37){
            sscanf(str.substr(i+1,2).c_str(), "%x", &tmp);
            res += (char)tmp;
            i = i + 2;
        }else res += str[i];
    }
    return res;
}

args_map parse_args(const char *args){
    args_map res;
    char *new_args = strdup(args);
    char *ptr = strtok(new_args, "&");
    while(ptr){
        char *tmp = strchr(ptr, '=');
        res[string(ptr, tmp)] = urldecode(string(tmp+1, ptr + strlen(ptr)));
        ptr = strtok(NULL, "&");
    }
    free(new_args);
    return res;
}

void body(const args_map &args){
    printf("<html>\n");
    printf("<head>\n");
    printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");
    printf("<title>Network Programming Homework 3</title>\n");
    printf("</head>\n");
    printf("<body bgcolor=#336699>\n");
    printf("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
    printf("<table width=\"800\" border=\"1\">\n");
    printf("<tr>\n");
    for(int i=1;i<=5;i++){
        if(args.find("h" + to_string(i)) != args.end() && args.at("h" + to_string(i)) != ""){
            printf("<td>%s</td>\n", args.at("h" + to_string(i)).c_str());
        }
    }
    printf("</tr>\n");
    printf("<tr>\n");
    for(int i=1;i<=5;i++){
        if(args.find("h" + to_string(i)) != args.end() && args.at("h" + to_string(i)) != ""){
            printf("<td valign=\"top\" id=\"m%d\"></td>\n", i);
        }
    }
    printf("</tr>\n");
    printf("</table>\n");
    printf("</font>");
    printf("</body>\n");
    printf("</html>\n");
    fflush(stdout);
}

void echo_msg(int id, const string &msg){
    printf("<script>");
    printf("document.all[\'m%d\'].innerHTML +=", id);
    printf("\"%s\";", msg.c_str());
    printf("</script>\n");
    fflush(stdout);
}
