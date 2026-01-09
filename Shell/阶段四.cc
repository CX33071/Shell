#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>
#include <signal.h>
using namespace std;
string current_pwd;
string old_pwd;
void init_shell();
void print_prompt();
vector<string> split_command(const string& cmd);
vector<vector<string>> split_pipe(const string& cmd);
bool parse_redirect(vector<string>& cmd_parts,
                    string& in_filoe,
                    string& out_file,
                    bool& is_append);
void execute_cd(const vector<string>& cmd_parts);
void execute_single_command(vector<string>& cmd_parts, bool& is_background);
void close_all_fds(const vector<int>& fds);
void execute_pipe_commands(const vector<vector<string>>& pipe_cmds,
                           string& in_file,
                           string& out_file,
                           bool& is_append,
                           bool& is_background);
void init_shell(){
    signal(SIGCHLD, SIG_IGN);
    char buf[PATH_MAX + 1] = {0};
    if(getcwd(buf,sizeof(buf))!=nullptr){
        current_pwd = buf;
        old_pwd = buf;
    }else{
        current_pwd = "unknown_dir";
        old_pwd = "unkno0wn_dir";
    }
}
void print_prompt(){
    cout << "my_shell";
    cout.flush();
}
vector<string> split_command(const string& cmd){
    vector<string> cmd_parts;
    string temp_part;
    for(char c:cmd){
        if(c==' '){
            if(!temp_part.empty()){
                cmd_parts.push_back(temp_part);
                temp_part.clear();
            }
        }else{
            temp_part += c;
        }
    }
    if(!temp_part.empty()){
        cmd_parts.push_back(temp_part);
    }
    return cmd_parts;
}
vector<vector<string>>split_pipe(const string& cmd){
    vector<vector<string>> pipe_cmds;
    string temp_cmd;
    for(char c:cmd){
        if(c=='|'){
            if(!temp_cmd.empty()){
                pipe_cmds.push_back(split_command(temp_cmd));
                temp_cmd.clear();
            }
        }else{
            temp_cmd += c;
        }
    }
    if(!temp_cmd.empty()){
        pipe_cmds.push_back(split_command(temp_cmd));
    }
    return pipe_cmds;
}
bool parse_redirect(vector<string>& cmd_parts,string& in_file,string& out_file,bool& is_append){
    vector<string> new_cmd_parts;
    in_file.clear();
    out_file.clear();
    is_append = false;
    for (int i = 0; i < cmd_parts.size(); ++i) {
        if(cmd_parts[i]=="<"){
            if(i+1>=cmd_parts.size()){
                cerr << "错误：输入重定向缺少文件名" << endl;
                return false;
            }
            in_file = cmd_parts[i + 1];
            i++;
        }else if(cmd_parts[i]==">"){
            if(i+1>=cmd_parts.size()){
                cerr << "错误：输出重定向缺少文件名" << endl;
                return false;
            }
            out_file = cmd_parts[i + 1];
            i++;
        }else if(cmd_parts[i]==">>"){
            if(i+1>=cmd_parts.size()){
                cerr << "错误：追加重定向缺少文件名" << endl;
                return false;
            }
            out_file = cmd_parts[i + 1];
            i++;
            is_append = true;
        }else{
            new_cmd_parts.push_back(cmd_parts[i]);
        }
    }
    cmd_parts.swap(new_cmd_parts);
    return true;
}
void execute_cd(const vector<string>& cmd_parts){

}
void execute_single_command(const vector<string>& cmd_parts,bool is_background){
    if(cmd_parts.empty()){
        return;
    }
    string in_file;
    string out_file;
    bool is_append;
    if(!parse_redirect(&cmd_parts,in_file,out_file,is_append)){
        return;
    }
    if(cmd_parts[0]=="cd"){
        execute_cd(cmd_parts);
        return;
    }
    pid_t child_pid = fork();
    if(child_pid==-1){
        cerr << "错误：创建子进程失败" << strerror(errno) << endl;
        return;
    }else if(child_pid==0){
        if(!in_file.empty()){
            int in_fd = open(in_file.c_str(), O_RDONLY);
            if(in_fd==-1){
                cerr << "错误：打开输入文件失败" << strerror(errno) << endl;
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if(!out_file.empty()){
            int open_flags = O_WRONLY | O_CREAT;
            if(is_append){
                open_flags|= O_APPEND;
            }else{
                open_flags |= O_TRUNC;
            }
            int out_fd = open(out_file.c_str(), open_flags, 0644);
            if(out_fd==-1){
                cerr << "错误：打开输出文件失败" << strerror(errno) << endl;
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        vector<char*> cmd_argv;
        for(const string& part:cmd_parts){
            cmd_argv.push_back(const_cast<char*>(part.c_str()));
        }
        cmd_argv.push_back(nullptr);
        execvp(cmd_argv[0], cmd_argv.data());
        cerr << "错误：命令执行失败" << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }else{
        if(!is_background){
            waitpid(child_pid, nullptr, 0);
        }else{
            cout << "后台进程PID:" << child_pid << endl;
        }
    }
}
void close_all_fds(const vector<int>& fds){
    for(int fd:fds){
        close(fd);
    }
    return;
}
void execute_pipe_command(const vector<vector<string>>& pipe_cmds,string& in_file,string& out_file,bool& is_append,bool& is_background){
    int cmd_count = pipe_cmds.size();
    if(cmd_count==0){
        return;
    }
    vector<int> pipe_fds;
    for (int i = 0; i < cmd_count - 1; ++i){
        int fd[2];
        if(pipe(fd)==-1){
            cerr << "错无：创建管道失败" << strerror(errno) << endl;
            close_all_fds(pipe_fds);
            return;
        }
        pipe_fds.push_back(fd[0]);
        pipe_fds.push_back(fd[1]);
    }
    for(int i=0;i<cmd_count;++i){
        pid_t child_pid = fork();
        if(child_pid==-1){
            cerr << "错误：创将管道子进程失败" << strerror(errno) << endl;
            close_all_fds(pipe_fds);
            return;
        }else if(child_pid==0){
            signal(SIGINT, SIG_DFL);
            if(i==0){
                if(!in_file.empty()){
                    int in_fd = open(in_file.c_str(), O_RDONLY);
                    if(in_fd==-1){
                        cerr << "错误：打开管道输入文件失败 "<< strerror(errno) << endl;
                        close_all_fds(pipe_fds);
                        exit(EXIT_FAILURE);
                    }
                    dup2(in_fd, STDIN_FILENO);
                    close(in_fd);
                }
            }else{
                int prev_pipe_read_fd =
                    pipe_fds[(i - 1) * 2]; 
                dup2(prev_pipe_read_fd, STDIN_FILENO);
            }
            if (i == cmd_count - 1) {
                if (!out_file.empty()) {
                    int open_flags = O_WRONLY | O_CREAT;
                    if (is_append) {
                        open_flags |= O_APPEND;
                    } else {
                        open_flags |= O_TRUNC;
                    }
                    int out_fd = open(out_file.c_str(), open_flags, 0644);
                    if (out_fd == -1) {
                        cerr << "错误：打开管道输出文件失败"
                             << strerror(errno) << endl;
                        close_all_fds(pipe_fds);
                        exit(EXIT_FAILURE);
                    }
                    dup2(out_fd, STDOUT_FILENO); 
                    close(out_fd);
                }
            } else {
                int curr_pipe_write_fd =
                    pipe_fds[i * 2 + 1]; 
                dup2(curr_pipe_write_fd,
                     STDOUT_FILENO); 
            }
            close_all_fds(pipe_fds);
            vector<string> curr_cmd = pipe_cmds[i];
            string dummy_in, dummy_out;
            bool dummy_append;
            if (!parse_redirect(curr_cmd, dummy_in, dummy_out, dummy_append)) {
                exit(EXIT_FAILURE);
            }
            if (curr_cmd.empty()) {
                exit(EXIT_SUCCESS);
            }
            vector<char*> cmd_argv;
            for (const string& part : curr_cmd) {
                cmd_argv.push_back(const_cast<char*>(part.c_str()));
            }
            cmd_argv.push_back(nullptr);
            execvp(cmd_argv[0], cmd_argv.data());
            cerr << "错误：管道命令执行失败 - " << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
    }
    close_all_fds(pipe_fds);
    if (!is_background) {
        for (int i = 0; i < cmd_count; ++i) {
            waitpid(-1, nullptr,
                    0); 
        }
    } else {
        cout << "[管道后台进程] 共 " << cmd_count << " 个子进程" << endl;
    }
}
int main(){
    init_shell();
    while(true){
        print_prompt();
        string cmd_line;
        if(!getline(cin,cmd_line)){
            cout << endl;
            break;
        }
        size_t start = cmd_line.find_first_not_of(' ');
        size_t end = cmd_line.find_last_not_of(' ');
        if (start == string::npos || end == string::npos) {
            continue;
        }
        cmd_line = cmd_line.substr(start, end - start + 1);
        bool is_background = false;
        if (cmd_line.back() == '&') {
            is_background = true;
            end = cmd_line.find_last_not_of('&');
            if (end == string::npos) {
                continue;
            }
            cmd_line = cmd_line.substr(0, end + 1);
            end = cmd_line.find_last_not_of(' ');
            if (end == string::npos) {
                continue;
            }
            cmd_line = cmd_line.substr(0, end + 1);
        }
        vector<vector<string>> pipe_cmds = split_pipe(cmd_line);

        if (pipe_cmds.size() == 1) {
            execute_single_command(pipe_cmds[0], is_background);
        } else {
            string in_file, out_file;
            bool is_append = false;
            vector<string> first_cmd = pipe_cmds[0];
            string dummy1, dummy2;
            bool dummy3;
            if (parse_redirect(first_cmd, in_file, dummy2, dummy3)) {
                pipe_cmds[0] = first_cmd;
            }
            vector<string> last_cmd = pipe_cmds.back();
            if (parse_redirect(last_cmd, dummy1, out_file, is_append)) {
                pipe_cmds.back() = last_cmd;
            }
            execute_pipe_commands(pipe_cmds, in_file, out_file, is_append,
                                  is_background);
        }
        if (!pipe_cmds.empty() && pipe_cmds[0].size() > 0 &&
            pipe_cmds[0][0] == "exit") {
            cout << "Bye Bye! 阶段4 Shell退出～" << endl;
            break;
        }
    }
    return 0;
}
