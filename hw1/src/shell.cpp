#include "shell.h"
#include "function.h"
#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "shell.h"

Shell::Shell(): command_num(0){}

void Shell::Init(){
	chdir("./ras");
	setenv("PATH", "bin:.", 1);
}

void Shell::Prompt(){
	printf("%% "); FSTDOUT;
}

void Shell::Run(){
	Init();
	char buf[BUFFER_SIZE];
	while(1){
		Prompt();
		fgets(buf, BUFFER_SIZE, stdin);
		int res = Exec(buf);
	}
}

int Shell::Exec(const std::string& buf){
	CommandLine cmd;
	if(cmd.Parse(buf, command_num) < 0) return -1;
	return _Exec(cmd);
}

int Shell::_Exec(CommandLine& cmd){
	if((int)cmd.size() == 0) return -1;
	if(cmd[0][0] == "exit"){
		exit(0);
	}else if(cmd[0][0] == "printenv"){
		if(cmd[0].size() < 2) return -1;
		printf("%s\n", getenv(cmd[0][1].c_str())); FSTDOUT;
	}else if(cmd[0][0] == "setenv"){
		if(cmd[0].size() < 3) return -1;
		setenv(cmd[0][1].c_str(), cmd[0][2].c_str(), 1);
	}else{
		cmd.Check();
		if((int)cmd.size() == 0) return -1;
		command_num++;
		return ExecCommand(cmd);
	}
}

int Shell::ExecCommand(const CommandLine& cmd){
	int pid, pgid = 0;
	for(int i=0;i<(int)cmd.size();i++){
		Command c = cmd[i];
		Pipe pipe_in, pipe_out, pipe_err;
		pipe_in = pipe_set[cmd.command_num][i];
		if(c.pipe_stdout != NON_PIPE)pipe_out = pipe_set[c.pipe_stdout.first][c.pipe_stdout.second];
		if(c.pipe_stderr != NON_PIPE)pipe_err = pipe_set[c.pipe_stderr.first][c.pipe_stderr.second];
		if((pid=fork()) == 0){
			if(dup2(pipe_in[0], 0) == -1)perror("dup2 stdin");
			if(c.pipe_stdout != NON_PIPE){
				if(dup2(pipe_out[1], STDOUT_FILENO) == -1)
					perror("dup2 stdout");
			}
			if(c.pipe_stderr != NON_PIPE){
				if(dup2(pipe_err[1], STDERR_FILENO) == -1)
					perror("dup2 stderr");
			}
			if(i == (int)cmd.size() - 1 && cmd.filename != ""){
				int fd = open(cmd.filename.c_str(), O_WRONLY|O_CREAT, 0660);
				if(dup2(fd, STDOUT_FILENO) == -1)
					perror("file redirect");
				close(fd);
			}
			for(int i=0;i<(int)cmd.size();i++){
				Command c = cmd[i];
				if(pipe_set.Find(cmd.command_num, i))pipe_set.Destruct(cmd.command_num, i);
				if(c.pipe_stdout != NON_PIPE && i != cmd.size()-1 && pipe_set.Find(c.pipe_stdout.first, c.pipe_stdout.second))
					pipe_set.Destruct(c.pipe_stdout.first, c.pipe_stdout.second);
				if(c.pipe_stderr != NON_PIPE && i != cmd.size()-1 && pipe_set.Find(c.pipe_stderr.first, c.pipe_stderr.second))
					pipe_set.Destruct(c.pipe_stderr.first, c.pipe_stderr.second);
			}
			MyExec(c);
		}else if(pid > 0){
			setpgid(pid, pgid);
			if(i == 0) pgid = pid;
			pipe_in.Destruct();
		}else{
		}
	}
	for(int i=0;i<(int)cmd.size();i++){
		Command c = cmd[i];
		if(pipe_set.Find(cmd.command_num, i))pipe_set.Destruct(cmd.command_num, i);
		if(c.pipe_stdout != NON_PIPE && i != cmd.size()-1 && pipe_set.Find(c.pipe_stdout.first, c.pipe_stdout.second))
			pipe_set.Destruct(c.pipe_stdout.first, c.pipe_stdout.second);
		if(c.pipe_stderr != NON_PIPE && i != cmd.size()-1 && pipe_set.Find(c.pipe_stderr.first, c.pipe_stderr.second))
			pipe_set.Destruct(c.pipe_stderr.first, c.pipe_stderr.second);
	}
	for(int i=0;i<(int)cmd.size();i++)
		waitpid(-pgid, NULL, 0);
}

int Shell::MyExec(const Command& cmd){
	const char *args[cmd.size()+2]={NULL};
	for(int i=0;i<(int)cmd.size();i++)
		args[i] = cmd[i].c_str();
	if(execvp(args[0], (char *const*)args) == -1)
		exit(errno);
	return 0;
}
