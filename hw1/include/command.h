#ifndef __COMMAND__H__
#define __COMMAND__H__
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define NON_PIPE std::pair<int, int>(-1, -1)
bool executable(const std::string&);

class Command{
public:
	std::vector<std::string> cmd;
	std::pair<int, int> pipe_stdout, pipe_stderr;
	Command();
	int size() const;
	std::string& operator [] (int);
	const std::string& operator [] (int) const;
	friend class CommandLine;
};


class CommandLine{
public:
	std::vector<Command> cmd;
	int pipe_stdout, pipe_stderr;
	int command_num;
	std::string filename;
	int size() const;
	CommandLine();
	int Parse(const std::string&, int);
	void Check();
	Command& operator [] (int);
	const Command& operator [] (int) const;

};

class Pipe{
public:
	std::vector<int> p;
	Pipe();
	~Pipe();
	void Create();
	void Destruct();
	int operator [] (int) const ;
};
class PipeLine{
public:
	std::map<int, Pipe> p;
	PipeLine();
	Pipe& operator [] (int);
};
class PipeSet{
public:
	std::map<int, PipeLine> p;
	PipeSet();
	PipeLine& operator [] (int);
};
#endif
