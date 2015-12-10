#define WM_SOCKET_SERVER    (WM_USER + 101)
#define WM_SOCKET_CLIENT    (WM_USER + 102)
#define WM_SOCKET_CGI       (WM_USER + 103)
#define BUF_SIZE 8192
#include <winsock2.h>
#include <bits/stdc++.h>
using namespace std;
typedef map<string, string> args_map;
struct cgi_meta{
    int id;
    SOCKET sock_fd;
    FILE *f;
    int write_enable;
    cgi_meta(){}
    cgi_meta(int _i, const SOCKET &_s, FILE *_f): id(_i), sock_fd(_s), f(_f), write_enable(0){}
};
map<SOCKET, cgi_meta> map_cgi_client;
map<SOCKET, int> map_client_count;
set<string> cgi = {".cgi"};

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
args_map parse_args(const char *);
void close_cgi(const SOCKET &);
void close_client(const SOCKET &);
bool has_prompt(const char *);
string urldecode(const string &);
string html_encode(const char *);
string escape(char);
void echo_msg(const cgi_meta &, const string &);
void echo_header(const SOCKET &);
void echo_body(const SOCKET &, const args_map &);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int show_cmd ) 
{ 
    WNDCLASS wc; 
    wc.style = CS_HREDRAW | CS_VREDRAW; 
    wc.lpfnWndProc = WindowProc; 
    wc.cbClsExtra = 0; 
    wc.cbWndExtra = 0; 
    wc.hInstance = hinstance; 
    wc.hIcon = LoadIcon(NULL, IDI_EXCLAMATION); 
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
    wc.lpszMenuName = NULL; 
    wc.lpszClassName = "np winsock"; 
    RegisterClass(&wc); 
    HWND hwnd = CreateWindow("np winsock", "", WS_SYSMENU, 300, 0, 600, 400, NULL, NULL, hinstance, NULL); 
    if (hwnd == NULL){ 
        MessageBox(hwnd, "create windows failed", "failed", MB_OK); 
        return 1; 
    } 
    ShowWindow(hwnd, SW_SHOWNORMAL); 
    UpdateWindow(hwnd); 
    WSADATA wsaData; 
    WORD version = MAKEWORD(2,0); 
    if (WSAStartup(version, &wsaData)!=0){ 
        MessageBox(NULL, "WSAStartup() Failed", "", 0); 
        return 1; 
    } 
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (s == INVALID_SOCKET){ 
        MessageBox(NULL, "socket() Failed", "", 0); 
        return 1; 
    }     
    sockaddr_in sin; 
    sin.sin_family = AF_INET; 
    sin.sin_port = htons(1111); 
    sin.sin_addr.S_un.S_addr = INADDR_ANY; 
    if (bind(s, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR){ 
        MessageBox(NULL, "bind() Failed", "", 0); 
        return 1; 
    } 
    if (listen(s, 3) == SOCKET_ERROR){ 
        MessageBox(NULL, "listen() Failed", "", 0); 
        return 1; 
    } 
	MessageBox(hwnd, "start listen", "", MB_OK); 
    WSAAsyncSelect(s, hwnd, WM_SOCKET_SERVER, FD_ACCEPT | FD_CLOSE); 
    MSG msg; 
    while (GetMessage(&msg, 0, 0, 0)){ 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    } 
    closesocket(s); 
    WSACleanup(); 
    return msg.wParam; 
} 
LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    if(umsg == WM_SOCKET_SERVER){
        SOCKET sock_fd = wparam;
        if(WSAGETSELECTERROR(lparam)){
            closesocket(sock_fd);
            return 0;
        }
        int event = WSAGETSELECTEVENT(lparam);
        if(event == FD_ACCEPT){
            sockaddr_in addr;
            int addr_len = sizeof(addr);
            SOCKET cli_fd = accept(sock_fd, (sockaddr*)&addr, &addr_len);
            if(cli_fd == INVALID_SOCKET || sock_fd == INVALID_SOCKET){
            }else{
                WSAAsyncSelect(cli_fd, hwnd, WM_SOCKET_CLIENT, FD_READ | FD_CLOSE);
            }
        }else if(event == FD_CLOSE){
            shutdown(sock_fd, SD_BOTH);
        }
    }else if(umsg == WM_SOCKET_CLIENT){
        SOCKET cli_fd = wparam;
        if(WSAGETSELECTERROR(lparam)){
            closesocket(cli_fd);
            return 0;
        }
        int event = WSAGETSELECTEVENT(lparam);
        if(event == FD_READ){
            char buf[BUF_SIZE] = {0};
            int ret = recv(cli_fd, buf, BUF_SIZE, 0);
            if((ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) || ret == 0){
                closesocket(cli_fd);
                return 0;
            }
            while(isspace(buf[ret - 1]))
                buf[--ret] = 0;
            if(strncmp(buf, "GET", 3) && strncmp(buf, "get", 3))
                return 0;
            char *ptr = strtok(buf, " ");
            ptr = strtok(NULL, " ");
            string path = string(ptr + 1);
            for(int i=0;i<path.size()-1;i++)
                if(path[i] == '.' && path[i+1] == '.')
                    return 0;
            string filename, query;
            if(path.find('?') == -1){
                filename= path.substr(path.find_last_of('/') + 1);
                query = "";
            }else{
                query = path.substr(path.find('?') + 1);
                path = path.substr(0, path.find('?'));
                filename= path.substr(path.find_last_of('/') + 1);
            }
            string ext;
            if(filename.find('.') != -1)
                ext = filename.substr(filename.find_last_of('.'));
            else ext = "";
            if(cgi.find(ext) == cgi.end()){
                int ret = send(cli_fd, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n", 63, 0);
                if(ret == SOCKET_ERROR || ret <= 0){
                    closesocket(cli_fd);
                    return 0;
                }
                FILE *f = fopen(path.c_str(), "rb");
                memset(buf, 0, sizeof(buf));
                while(fgets(buf, BUF_SIZE, f)){
                    string tmp = string(buf) + "\r\n";
                    send(cli_fd, buf, sizeof(buf), 0);
                    if(ret == SOCKET_ERROR || ret <= 0){
                        closesocket(cli_fd);
                        return 0;
                    }
                    fprintf(stderr, "%s", buf);
                    memset(buf, 0, sizeof(buf));
                }
                fclose(f);
                shutdown(cli_fd, SD_BOTH);
            }else{
                args_map args = parse_args(query.c_str());
                echo_header(cli_fd);
                echo_body(cli_fd, args);
                map_client_count[cli_fd] = 0;
                for(int i=1;i<=5;i++){
                    if(args.find("h" + to_string(i)) != args.end() && args.at("h" + to_string(i)) != ""){
                        FILE *f = fopen(args.at("f" + to_string(i)).c_str(), "r");
                        if(f == NULL){
                            continue;
                        }
                        SOCKET cgi_fd;
                        sockaddr_in addr;
                        ZeroMemory(&addr, sizeof(addr));
                        if((cgi_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
                            continue;
                        }
                        unsigned long non_blocking = 1;
                        if(ioctlsocket(cgi_fd, FIONBIO, &non_blocking) == SOCKET_ERROR){
                            continue;
                        }
                        hostent *host = gethostbyname(args.at("h" + to_string(i)).c_str());
                        addr.sin_addr.s_addr = *(u_long*)host->h_addr;
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(stoi(args.at("p" + to_string(i))));
                        if(connect(cgi_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR){
                            int err = WSAGetLastError();
                            if(!(err == 10035 || err == 10056))
                                continue;
                        }
                        map_cgi_client[cgi_fd] = cgi_meta(i, cli_fd, f);
                        map_client_count[cli_fd]++;
                        WSAAsyncSelect(cgi_fd, hwnd, WM_SOCKET_CGI, FD_READ | FD_CLOSE);
                    }
                }
                if(map_client_count[cli_fd] == 0)
                    close_client(cli_fd);
            }
        }else if(event == FD_CLOSE){
            close_client(cli_fd);
        }
    }else if(umsg == WM_SOCKET_CGI){
        SOCKET cgi_fd = wparam;
        if(WSAGETSELECTERROR(lparam)){
            close_cgi(cgi_fd);
            return 0;
        }
        int event = WSAGETSELECTEVENT(lparam);
        cgi_meta &meta = map_cgi_client[cgi_fd];
        if(event == FD_READ){
            char buf[BUF_SIZE] = {0};
            int ret = recv(cgi_fd, buf, BUF_SIZE, 0);
            if(ret == SOCKET_ERROR || ret <= 0){
                close_cgi(cgi_fd);
                return 0;
            }
            echo_msg(meta, html_encode(buf));
            if(has_prompt(buf)) {
                meta.write_enable = 1;
                if(fgets(buf, BUF_SIZE, meta.f) == NULL){
                    close_cgi(cgi_fd);
                    return 0;
                }
                echo_msg(meta, "<b>" + html_encode(buf) + "</b>");
                int ret = send(cgi_fd, buf, strlen(buf), 0);
                if(ret == SOCKET_ERROR || ret <= 0){
                    close_cgi(cgi_fd);
                    return 0;
                }
            }
        }else if(event == FD_CLOSE){
            close_cgi(cgi_fd);
        }
    }else if(umsg == WM_CLOSE){
        if (MessageBox(hwnd, "exit?", "message", MB_YESNO) == IDYES) 
            DestroyWindow(hwnd); 
    }else if(umsg == WM_DESTROY){
        PostQuitMessage(0);
    }else{
        return DefWindowProc(hwnd,umsg,wparam,lparam); 
    }
    return 0; 
} 

string html_encode(const char *buf){
    string res = "";
    int len = strlen(buf);
    for(int i=0;i<len;i++){
        res += escape(buf[i]);
    }
    return res;
}

string escape(char c){
    if(c == '\n') return "<br>";
    if(c == '\r') return "";
    if(c == ' ') return "&nbsp;";
    if(c == '\'') return "\\\'";
    if(c == '\"') return "\\\"";
    if(c == '<') return "&lt;";
    if(c == '>') return "&gt;";
    return string(1, c);
}

bool has_prompt(const char *buf){
    int len = strlen(buf);
    for(int i=0;i<len;i++){
        if(buf[i] == '%')
            return true;
    }
    return false;
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

void close_cgi(const SOCKET &fd){
    map_client_count[map_cgi_client[fd].sock_fd]--;
    if(map_client_count[map_cgi_client[fd].sock_fd] == 0)
        close_client(map_cgi_client[fd].sock_fd);
    map_cgi_client.erase(fd);
    shutdown(fd, SD_BOTH);
}

void close_client(const SOCKET &fd){
    map_client_count.erase(fd);
    shutdown(fd, SD_BOTH);
}

void echo_header(const SOCKET &fd){
    const char *buf = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(fd, buf, strlen(buf), 0);
}

void echo_body(const SOCKET &fd, const args_map &args){
    char buf[BUF_SIZE] = {0};
    const char *buf1 = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" /><title>Network Programming Homework 3</title></head><body bgcolor=#336699><font face=\"Courier New\" size=2 color=#FFFF99><table width=\"800\" border=\"1\"><tr>";
    send(fd, buf1, strlen(buf1), 0);
    for(int i=1;i<=5;i++){
        if(args.find("h" + to_string(i)) != args.end() && args.at("h" + to_string(i)) != ""){
            sprintf(buf, "<td>%s</td>\n", args.at("h" + to_string(i)).c_str());
            send(fd, buf, strlen(buf), 0);
        }
    }
    const char *buf2 = "</tr><tr>";
    send(fd, buf2, strlen(buf2), 0);
    for(int i=1;i<=5;i++){
        if(args.find("h" + to_string(i)) != args.end() && args.at("h" + to_string(i)) != ""){
            sprintf(buf, "<td valign=\"top\" id=\"m%d\"></td>\n", i);
            send(fd, buf, strlen(buf), 0);
        }
    }
    const char *buf3 = "</tr></table></body></html>";
    send(fd, buf3, strlen(buf3), 0);
}

void echo_msg(const cgi_meta &meta, const string &msg){
    const char *t_buf = "<script>document.all[\'m%d\'].innerHTML +=\"%s\";</script>";
    char buf[BUF_SIZE] = {0};
    sprintf(buf, t_buf, meta.id, msg.c_str());
    send(meta.sock_fd, buf, strlen(buf), 0);
}