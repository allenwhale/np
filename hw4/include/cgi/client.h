#ifndef __CLIENT__H__
#define __CLIENT__H__
#include "hw4.h"
#include <bits/stdc++.h>
#define C_CONNECTING1 1
#define C_CONNECTING2 4
#define C_WRITING 2
#define C_READING 3
#define BUF_SIZE 1024*32
using namespace std;
void set_nonblock(int);
void run(const args_map &);
string html_encode(const char *);
string escape(char);
bool has_prompt(const char *);
#endif
