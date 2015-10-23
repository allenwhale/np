#ifndef __SHELL__H__
#define __SHELL__H__
#include "command.h"
#include <bits/stdc++.h>
extern char **environ;
class Shell{
public:
	int command_num;
	PipeSet pipe_set;
	Shell();
	void Init();
	void Run();
	void Prompt();
	int Exec(const std::string&);
	int _Exec(CommandLine&);
	int ExecCommand(const CommandLine&);
	int MyExec(const Command&);
	void Exit();
	int Communicate(const std::string&);
	void PrintEnv();
};
#endif
