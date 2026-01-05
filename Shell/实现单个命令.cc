#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
void print();
vector<string> cut_mand(const string& cmd);
void execute_single_command(const vector<string>& cmd_parts);
void print() {
    cout << "my_shell";
    cout.flush();
}
vector<string> cut_mand(const string& cmd) {
    vector<string> cmd_parts;
    string temp_part;
    for (char c : cmd) {
        if (c == ' ') {
            if (!temp_part.empty()) {
                cmd_parts.push_back(temp_part);
                temp_part.clear();
            }
        } else {
            temp_part += c;
        }
    }
    if (!temp_part.empty()) {
        cmd_parts.push_back(temp_part);
    }
    return cmd_parts;
}
void execute_single_command(const vector<string>& cmd_parts) {
    if (cmd_parts.empty()) {
        return;
    }
    if (cmd_parts[0] == "cd") {
        cout << "cd命令为内置命令，后续完善" << endl;
        return;
    }
    pid_t child_pid = fork();
    if (child_pid == -1) {
        cerr << "错误：创建子进程失败" << strerror(errno) << endl;
        return;
    } else if (child_pid == 0) {
        vector<char*> cmd_argv;
        for (const string& part : cmd_parts) {
            cmd_argv.push_back(const_cast<char*>(part.c_str()));
        }
        cmd_argv.push_back(nullptr);
        execvp(cmd_argv[0], cmd_argv.data());
        cerr << "错误：命令执行错误" << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    } else {
        waitpid(child_pid, nullptr, 0);
    }
}
int main() {
    while (true) {
        print();
        string cmd_line;
        if (!getline(cin, cmd_line)) {
            cout << endl;
            break;
        }
        vector<string> cmd_parts = cut_mand(cmd_line);
        if (!cmd_parts.empty() && cmd_parts[0] == "exit") {
            cout << "bye bye" << endl;
            break;
        }
        execute_single_command(cmd_parts);
    }
    return 0;
}