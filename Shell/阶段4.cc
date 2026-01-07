#include <iostream>
#include <vector>
#include <string>
#include <cstring>
using namespace std;
vector<string> split_command(const string& cmd) {}
vector<vector<string>> split_pipe(const string& cmd) {
    vector<vecotr<string>> pipe_cmds;
    string temp_cmd;
    for (char c : cmd) {
        if (c == '|') {
            if (!temp_cmd.empty()) {
                pipe_cmds.push_back(split_command(temp_cmd));
                temp_cmd.clear();
            }
        } else {
            temp_cmd += c;
        }
    }
    if (!temp_cmd.empty()) {
        pipe_cmds.push_back(split_command(temp_cmd));
    }
    return pipe_cmds;
}