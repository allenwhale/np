#include "shell.h"
#include "function.h"
#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include "shell.h"
#include "server.h"
#include "sem.h"

Shell::Shell(): command_num(0){
	Init();
}

void Shell::Init(){
	chdir("./ras");
	setenv("PATH", "bin:.", 1);
	puts("****************************************");
	puts("** Welcome to the information server. **");
	puts("****************************************");
}

void Shell::Prompt(){
	printf("%% "); FSTDOUT;
}

void Shell::Run(int cli_id, int server_shm_id){
    server = (Server*)shmat(server_shm_id, (void*)0, 0);
    set_nonblock(STDIN_FILENO);
    id = cli_id;
	char buf[BUFFER_SIZE];
    int res = -1;
	while(1){
        server->msg[id].Flush();
        if(res == INTERNAL_COMMAND) Prompt();
        memset(buf, 0, sizeof(buf));
        res = read(STDIN_FILENO, buf, BUFFER_SIZE);
        if(res == 0){
            server->clients[id].used = false;
            char msg[256] = {0};
            sprintf(msg, "*** User '%s' left. ***\n", server->clients[id].nickname);
            printf("%s", msg);
            for(int i=1;i<MAX_CLIENT_SIZE;i++)
                if(server->clients[i].used && i != id)
                    server->msg[i].Append(msg);
            shmdt(server);
            exit(0);
        }else if(res < 0){
            usleep(1000);
            continue;
        }else{
            res = Exec(buf);
            if(res != INTERNAL_COMMAND)Prompt();
        }
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
        server->clients[id].used = false;
        char msg[256] = {0};
        sprintf(msg, "*** User '%s' left. ***\n", server->clients[id].nickname);
        printf("%s", msg);
        for(int i=1;i<MAX_CLIENT_SIZE;i++)
            if(server->clients[i].used && i != id)
                server->msg[i].Append(msg);
        shmdt(server);
        exit(0);
    }else if(cmd[0][0] == "printenv"){
		if(cmd[0].size() < 2) return -1;
		printf("%s=%s\n", cmd[0][1].c_str(), getenv(cmd[0][1].c_str())); FSTDOUT;
	}else if(cmd[0][0] == "setenv"){
		if(cmd[0].size() < 3) return -1;
		setenv(cmd[0][1].c_str(), cmd[0][2].c_str(), 1);
    }else if(cmd[0][0] == "who"){
        server->msg[id].Append("<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
        for(int i=1;i<MAX_CLIENT_SIZE;i++){
            if(server->clients[i].used){
                char msg[256] = {0};
                sprintf(msg, "%d\t%s\t%s/%d%s\n", i, server->clients[i].nickname, server->clients[i].ip, server->clients[i].port, (i==id)?"\t<-me":"");
                server->msg[id].Append(msg);
            }
        }
        return INTERNAL_COMMAND;
    }else if(cmd[0][0] == "name"){
        if(cmd[0].size() < 2) return -1;
        if(server->CheckNickname(cmd[0][1].c_str()) == false){
            char msg[256] = {0};
            sprintf(msg, "*** User '%s' already exists. ***\n", cmd[0][1].c_str());
            server->msg[id].Append(msg);
        }else{
            server->clients[id].set_nickname(cmd[0][1].c_str());
            char msg[256] = {0};
            sprintf(msg, "*** User from %s/%d is named '%s'. ***\n", server->clients[id].ip, server->clients[id].port, server->clients[id].nickname);
            for(int i=1;i<MAX_CLIENT_SIZE;i++)
                if(server->clients[i].used)
                    server->msg[i].Append(msg);
        }
        return INTERNAL_COMMAND;
    }else if(cmd[0][0] == "tell"){
        if(cmd[0].size() < 3) return -1;
        std::string buf = cmd.origin;
        buf = buf.substr(buf.find(' ')+1);
        std::string str_to_id = buf.substr(0, buf.find(' '));
        int to_id = string2int(str_to_id);
        buf = buf.substr(buf.find(' ')+1);
        if(server->clients[to_id].used == false){
            char msg[256] = {0};
            sprintf(msg, "*** Error: user #%d does not exist yet. ***\n", to_id);
            server->msg[id].Append(msg);
        }else{
            char msg[256] = {0};
            sprintf(msg, "*** %s told you ***: %s\n", server->clients[id].nickname, buf.c_str());
            server->msg[to_id].Append(msg);
        }
        return INTERNAL_COMMAND;
    }else if(cmd[0][0] == "yell"){
        if(cmd[0].size() < 2) return -1;
        char msg[256] = {0};
        sprintf(msg, "*** %s yelled ***: %s\n", server->clients[id].nickname, cmd.origin.substr(cmd.origin.find(' ')+1).c_str());
        for(int i=1;i<MAX_CLIENT_SIZE;i++)
            if(server->clients[i].used)
                server->msg[i].Append(msg);
        return INTERNAL_COMMAND;
    }else{
		cmd.Check(server);
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
			if(dup2(pipe_in[0], STDIN_FILENO) == -1)perror("dup2 stdin");
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
            if(c.global_pipe_stdin != -1){
                char msg[256] = {0};
                sprintf(msg, "*** %s (#%d) just received via '%s' ***\n", server->clients[id].nickname, id, cmd.origin.c_str());
                printf("%s", msg);
                FSTDOUT;
                for(int i=1;i<MAX_CLIENT_SIZE;i++)
                    if(server->clients[i].used && i != id)
                        server->msg[i].Append(msg);
                int fd = open(("/tmp/np.global_pipe" + std::to_string(c.global_pipe_stdin)).c_str(), O_RDONLY);
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if(c.global_pipe_stdout != -1){
                char msg[256] = {0};
                sprintf(msg, "*** %s (#%d) just piped '%s' ***\n", server->clients[id].nickname, id, cmd.origin.c_str());
                printf("%s", msg);
                FSTDOUT;
                for(int i=1;i<MAX_CLIENT_SIZE;i++)
                    if(server->clients[i].used && i != id)
                        server->msg[i].Append(msg);
                int fd = open(("/tmp/np.global_pipe" + std::to_string(c.global_pipe_stdout)).c_str(), O_WRONLY|O_CREAT, 0660);
                ftruncate(fd, 0);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
			for(int i=0;i<(int)cmd.size();i++){
				Command c = cmd[i];
				if(pipe_set.Find(cmd.command_num, i))
                    pipe_set.Destruct(cmd.command_num, i);
				if(c.pipe_stdout != NON_PIPE && i != cmd.size()-1)
					pipe_set.Destruct(c.pipe_stdout.first, c.pipe_stdout.second);
				if(c.pipe_stderr != NON_PIPE && i != cmd.size()-1)
					pipe_set.Destruct(c.pipe_stderr.first, c.pipe_stderr.second);
			}
			MyExec(c);
		}else if(pid > 0){
			setpgid(pid, pgid);
			if(i == 0) pgid = pid;
            pipe_set.Destruct(cmd.command_num, i);
		}else{
            puts("failed");
		}
	}
	for(int i=0;i<(int)cmd.size();i++){
		Command c = cmd[i];
		if(pipe_set.Find(cmd.command_num, i))pipe_set.Destruct(cmd.command_num, i);
		if(c.pipe_stdout != NON_PIPE && i != cmd.size()-1)
			pipe_set.Destruct(c.pipe_stdout.first, c.pipe_stdout.second);
		if(c.pipe_stderr != NON_PIPE && i != cmd.size()-1)
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

void Shell::Communicate(const std::string &msg){
    flock(fifofd, LOCK_EX);
    write(fifofd, msg.c_str(), msg.size());
    flock(fifofd, LOCK_UN);
}
