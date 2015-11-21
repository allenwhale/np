#ifndef __SHELL__H__
#define __SHELL__H__
#include "command.h"
//#include "client.h"
#include <map>
#include <bits/stdc++.h>
#define SHELL_EXIT -10
extern char **environ;
class Client;
class Server;
class Shell{
public:
	int command_num;
	PipeSet pipe_set;
    std::map<std::string, std::string> env;
	Shell();
	void Init();
	int Run(Client*, Server*);
	void Prompt();
	int Exec(const std::string&, Client*, Server*);
	int _Exec(CommandLine&, Client*, Server*);
	int ExecCommand(const CommandLine&, Client*, Server*);
	int MyExec(const Command&);
	void Exit();
	int Communicate(const std::string&);
	void PrintEnv();
};
#endif
