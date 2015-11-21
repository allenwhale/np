#include "command.h"
#include "function.h"
#include "server.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <bits/stdc++.h>

bool executable(const std::string& str){
	char *s = strdup(getenv("PATH"));
	std::vector<std::string> path;
	char *ptr = strtok(s, ":");
	while(ptr){
		path.push_back(ptr);
		ptr = strtok(NULL, ":");
	}
	for(auto p: path){
		std::string abs_path = p + "/" + str;
		struct stat s;
		if(((stat(abs_path.c_str(), &s) >= 0) && (s.st_mode > 0) && (S_IEXEC & s.st_mode)))
			return true;
	}
	return false;
}

Command::Command(): pipe_stderr(NON_PIPE), pipe_stdout(NON_PIPE), global_pipe_stdin(-1), global_pipe_stdout(-1){}

int Command::size() const {
	return cmd.size();
}

std::string& Command::operator [] (int n){
	return cmd[n];
}

const std::string& Command::operator [] (int n) const {
	return cmd[n];
}

CommandLine::CommandLine(): pipe_stdout(0), pipe_stderr(0), filename(""){}

int CommandLine::size() const{
	return cmd.size();
}

int CommandLine::Parse(const std::string& str, int cmd_num){
	command_num = cmd_num;
	cmd.clear();
    origin = str;
    while(isspace(origin.back()))origin.pop_back();
	std::vector<std::string> split;
	std::stringstream ss(str);
	std::string s;
	while(ss >> s) split.push_back(s);
	int new_command = 1;
	if((int)split.size() > 2 && split[split.size()-2] == ">"){
		filename = split.back();
		split.pop_back();
		split.pop_back();
	}
	int num = 1;
	for(auto c: split){
		if(c[0] == '|'){
			num = (int)c.size() > 1 ? string2int(c.substr(1)) : 1;
			cmd.back().pipe_stdout = {cmd_num, cmd.size() - 1 + num};
			new_command = 1;
		}else if(c[0] == '!'){
			num = (int)c.size() > 1 ? string2int(c.substr(1)) : 1;
			if(c.size() > 1) num = string2int(c.substr(1));
			cmd.back().pipe_stderr = {cmd_num, cmd.size() - 1 + num};
			new_command = 1;
        }else if(c[0] == '>'){
            cmd.back().global_pipe_stdout = string2int(c.substr(1));
            new_command = 1;
        }else if(c[0] == '<'){
            cmd.back().global_pipe_stdin = string2int(c.substr(1));
            new_command = 1;
		}else{
			if(new_command) cmd.push_back(Command());
			new_command = 0;
			cmd.back().cmd.push_back(c);
		}
	}
	if(cmd.size() && cmd.back().pipe_stdout != NON_PIPE)
		cmd.back().pipe_stdout = {cmd_num + num, 0};
	if(cmd.size() && cmd.back().pipe_stderr != NON_PIPE)
		cmd.back().pipe_stderr = {cmd_num + num, 0};
	return 0;
}
void CommandLine::Check(Server *server){
	for(int i=0;i<(int)cmd.size();i++){
		if(executable(cmd[i][0]) == false){
			printf("Unknown command: [%s].\n", cmd[i][0].c_str()); FSTDOUT;
			for(int j=0;j<cmd.size()-i+1;j++) cmd.pop_back();
			filename = "";
			break;
		}
    }
    for(int i=0;i<(int)cmd.size();i++){
        if(cmd[i].global_pipe_stdin != -1 && !server->global_pipe[cmd[i].global_pipe_stdin]){
            printf("*** Error: the pipe #%d does not exist yet. ***\n", cmd[i].global_pipe_stdin);
            for(int j=0;j<cmd.size()-i+1;j++) cmd.pop_back();
            break;
        }
    }
    for(int i=0;i<(int)cmd.size();i++){
        if(cmd[i].global_pipe_stdout != -1 && server->global_pipe[cmd[i].global_pipe_stdout]){
            printf("*** Error: the pipe #%d already exists. ***\n", cmd[i].global_pipe_stdout);
            for(int j=0;j<cmd.size()-i+1;j++) cmd.pop_back();
            break;
        }
    }
    for(int i=0;i<(int)cmd.size();i++){
        if(cmd[i].global_pipe_stdin != -1){
            server->global_pipe[cmd[i].global_pipe_stdin] = false;
        }
        if(cmd[i].global_pipe_stdout != -1){
            server->global_pipe[cmd[i].global_pipe_stdout] = true;
        }
    }
}
Command& CommandLine::operator [] (int n){
	return cmd[n];
}
const Command& CommandLine::operator [] (int n) const {
	return cmd[n];
}

Pipe::Pipe(): p(std::vector<int>(2)){}
Pipe::~Pipe(){}
void Pipe::Destruct(){
	close(p[0]); close(p[1]);
}
void Pipe::Create(){
	int _p[2];
	pipe(_p);
	p[0] = _p[0], p[1] = _p[1];
}
int Pipe::operator [] (int n) const {
	return p[n];
}

PipeLine::PipeLine(){}
Pipe& PipeLine::operator [] (int n){
	if(p.find(n) == p.end()) p[n] = Pipe(), p[n].Create();
	return p[n];
}

PipeSet::PipeSet(){}
PipeLine& PipeSet::operator [] (int n){
	if(p.find(n) == p.end()) p[n] = PipeLine();
	return p[n];
}
bool PipeSet::Find(int x, int y) {
	if(p.find(x) == p.end()) return false;
	if(p[x].p.find(y) == p[x].p.end()) return false;
	return true;
}
void PipeSet::Destruct(int x, int y){
	if(Find(x, y) == false) return;
	(*(this))[x][y].Destruct();
	(*(this))[x].p.erase(y);
}
