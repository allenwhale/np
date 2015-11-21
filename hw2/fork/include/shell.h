#ifndef __SHELL__H__
#define __SHELL__H__
#include "command.h"
#include <bits/stdc++.h>
#define INTERNAL_COMMAND -100
class Server;
extern char **environ;
class Shell{
public:
	int command_num;
	PipeSet pipe_set;
    int id, fifofd;
    Server *server;
	Shell();
	void Init();
	void Run(int, int);
	void Prompt();
	int Exec(const std::string&);
	int _Exec(CommandLine&);
	int ExecCommand(const CommandLine&);
	int MyExec(const Command&);
	void Exit();
	void Communicate(const std::string&);
	void PrintEnv();
};
#endif
