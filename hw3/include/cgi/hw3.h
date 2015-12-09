#ifndef __HW3__H__
#define __HW3__H__
#include <bits/stdc++.h>
using namespace std;
typedef map<string, string> args_map;
void header();
args_map parse_args(const char *);
string urldecode(const string &);
void body(const args_map&);
void echo_msg(int, const string &);
#endif
