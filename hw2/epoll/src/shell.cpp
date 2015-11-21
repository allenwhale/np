#include "shell.h"
#include "function.h"
#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "shell.h"
#include <map>
#include "client.h"
#include "server.h"
#include "command.h"
class Server;
Shell::Shell(): command_num(0){
    Init();
}

void Shell::Init(){
	chdir("./ras");
    env["PATH"] = "bin:.";
	puts("****************************************");
	puts("** Welcome to the information server. **");
	puts("****************************************");
    FSTDOUT;
}

void Shell::Prompt(){
	printf("%% "); FSTDOUT;
}

int Shell::Run(Client* client, Server* server){
	char buf[BUFFER_SIZE];
    if(fgets(buf, BUFFER_SIZE, stdin) == NULL)
        return SHELL_EXIT;
    buf[strlen(buf)-1] = 0;
    int res = Exec(buf, client, server);
    if(res != SHELL_EXIT)Prompt();
    return res;
}

int Shell::Exec(const std::string& buf, Client* client, Server* server){
	CommandLine cmd;
	if(cmd.Parse(buf, command_num) < 0) return -1;
	return _Exec(cmd, client, server);
}

int Shell::_Exec(CommandLine& cmd, Client* client, Server* server){
	if((int)cmd.size() == 0) return -1;
	if(cmd[0][0] == "exit"){
        return SHELL_EXIT;
	}else if(cmd[0][0] == "printenv"){
		if(cmd[0].size() < 2) return -1;
		printf("%s=%s\n", cmd[0][1].c_str(), env[cmd[0][1]].c_str()); FSTDOUT;
	}else if(cmd[0][0] == "setenv"){
		if(cmd[0].size() < 3) return -1;
        env[cmd[0][1]] = cmd[0][2];
    }else if(cmd[0][0] == "yell"){
        if(cmd[0].size() < 2) return -1;
        char msg[256];
        sprintf(msg, "*** %s yelled ***: %s\n", client->nickname.c_str(), cmd.origin.substr(cmd.origin.find(' ')+1).c_str());
        server->SendAll(msg);
    }else if(cmd[0][0] == "tell"){
        if(cmd[0].size() < 3) return -1;
        if(server->id2client.find(stoi(cmd[0][1])) == server->id2client.end()){
            printf("*** Error: user #%s does not exist yet. ***\n", cmd[0][1].c_str());
            return -1;
        }
        char msg[256];
        sprintf(msg, "*** %s told you ***: %s\n", client->nickname.c_str(), cmd.origin.substr(cmd.origin.find(' ', cmd.origin.find(' ')+1)+1).c_str());
        server->Send(server->id2client[stoi(cmd[0][1])]->fd, msg);
    }else if(cmd[0][0] == "name"){
        if(cmd[0].size() < 2) return -1;
        if(server->nickname.find(cmd[0][1]) != server->nickname.end()){
            printf("*** User '%s' already exists. ***\n", cmd[0][1].c_str());
            return -1;
        }
        char msg[256];
        server->nickname.erase(client->nickname);
        client->nickname = cmd[0][1];
        sprintf(msg, "*** User from %s/%d is named '%s'. ***\n", client->ip.c_str(), client->port, client->nickname.c_str());
        server->nickname.insert(client->nickname);
        server->SendAll(msg);
    }else if(cmd[0][0] == "who"){
        printf("<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
        for(auto cli: server->clients){
            printf("%d\t%s\t%s/%d%s\n", cli->id, cli->nickname.c_str(), cli->ip.c_str(), cli->port, (cli->id==client->id)?"\t<-me":"");
        }
	}else{
		cmd.Check(server, client->shell->env["PATH"]);
		if((int)cmd.size() == 0) return -1;
		command_num++;
		return ExecCommand(cmd, client, server);
	}
    return 0;
}

int Shell::ExecCommand(const CommandLine& cmd, Client* client, Server* server){
	int pid, pgid = 0;
    setenv("PATH", client->shell->env["PATH"].c_str(), 1);
	for(int i=0;i<(int)cmd.size();i++){
		Command c = cmd[i];
		Pipe pipe_in, pipe_out, pipe_err, global_pipe_in, global_pipe_out;;
		pipe_in = pipe_set[cmd.command_num][i];
		if(c.pipe_stdout != NON_PIPE)pipe_out = pipe_set[c.pipe_stdout.first][c.pipe_stdout.second];
		if(c.pipe_stderr != NON_PIPE)pipe_err = pipe_set[c.pipe_stderr.first][c.pipe_stderr.second];
        if(c.global_pipe_stdin != -1)global_pipe_in = server->global_pipe[c.global_pipe_stdin];
        if(c.global_pipe_stdout != -1)global_pipe_in = server->global_pipe[c.global_pipe_stdout];
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
            if(c.global_pipe_stdin != -1){
                auto& p = server->global_pipe[c.global_pipe_stdin];
                if(dup2(p[0], STDIN_FILENO) == -1)
                    perror("global pipe stdin error");
                char msg[256];
                sprintf(msg, "*** %s (#%d) just received via \'%s\' ***\n", client->nickname.c_str(), client->id, cmd.origin.c_str());
                server->SendAll(msg);
                server->global_pipe.Destruct(c.global_pipe_stdin);
            }
            if(c.global_pipe_stdout != -1){
                auto& p = server->global_pipe[c.global_pipe_stdout];
                if(dup2(p[1], STDOUT_FILENO) == -1)
                    perror("global pipe stdout error");
                char msg[256];
                sprintf(msg, "*** %s (#%d) just piped '%s' ***\n", client->nickname.c_str(), client->id, cmd.origin.c_str());
                server->SendAll(msg);
                server->global_pipe.Destruct(c.global_pipe_stdout);
            }
			if(i == (int)cmd.size() - 1 && cmd.filename != ""){
				int fd = open(cmd.filename.c_str(), O_WRONLY|O_CREAT, 0660);
				if(dup2(fd, STDOUT_FILENO) == -1)
					perror("file redirect");
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
            if(c.global_pipe_stdin != -1)
                server->global_pipe.Destruct(c.global_pipe_stdin);
		}else{
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
	const char *args[cmd.size()+2] = {NULL};
	for(int i=0;i<(int)cmd.size();i++)
		args[i] = cmd[i].c_str();
	if(execvp(args[0], (char *const*)args) == -1){
		exit(errno);
    }
	return 0;
}
