#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cerrno>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
using namespace std;
string old_pwd;
string current_pwd;
void init_shell();
void print_prompt();
vector<string> split_command(const string& cmd);
bool parse_redirect(vector<string>& cmd_parts,
                    string& in_file,
                    string& out_file,
                    bool& is_append);
void execute_cd(const vector<string>& cmd_parts);
void execute_single_command(vector<string> cmd_parts, bool is_background);
void init_shell(){
    signal(SIGCHLD, SIG_IGN);
    char buf[PATH_MAX + 1] = {0};
    if(getcwd(buf,sizeof(buf))!=nullptr){
        current_pwd = buf;
        old_pwd = buf;
    }else{
        current_pwd = "unkown_dir";
        old_pwd = "unkown_dir";
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
bool parse_redirect(vector<string>& cmd_parts,string& in_file,string& out_file,bool& is_append){
    in_file.clear();
    out_file.clear();
    is_append = false;
    vector<string> new_cmd_parts;
    for(size_t i=0;i<cmd_parts.size();++i){
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
            is_append = false;
            i++;
        }else if(cmd_parts[i]==">>"){
            if(i+1>=cmd_parts.size()){
                cerr << " 错误：追加重定向缺少文件名 "<<endl;
                return false;
            }
            out_file = cmd_parts[i + 1];
            is_append = true;
            i++;
        }else{
            new_cmd_parts.push_back(cmd_parts[i]);
        }
    }
    cmd_parts.swap(new_cmd_parts);
    return true;
}
void execute_cd(const vector<string>& cmd_parts) {
    string target_dir; 
    if (cmd_parts.size() == 1) {
        char* home_dir = getenv("HOME");
        if (home_dir == nullptr) {
            cerr << "错误：无法获取主目录（HOME环境变量未设置）！" << endl;
            return;
        }
        target_dir = home_dir;
    } else if (cmd_parts[1] == "-") {
        if (old_pwd.empty() || old_pwd == "unknown_dir") {
            cerr << "错误：无历史工作目录记录！" << endl;
            return;
        }
        target_dir = old_pwd;
        cout << target_dir << endl; 
    } else {
        target_dir = cmd_parts[1];
    }
    string temp_old_dir = current_pwd;
    if (chdir(target_dir.c_str()) == -1) {
        cerr << "错误：切换目录失败 - " << strerror(errno) << endl;
        return; 
    }
    old_pwd = temp_old_dir;  
    char buf[PATH_MAX + 1] = {0};
    if (getcwd(buf, sizeof(buf)) != nullptr) {
        current_pwd = buf;                     
        setenv("PWD", current_pwd.c_str(), 1); 
    }
}
void execute_single_command(vector<string> cmd_parts,bool is_background){
    if(cmd_parts.empty()){
        return;
    }
    string in_file;
    string out_file;
    bool is_append;
    if(!parse_redirect(cmd_parts,in_file,out_file,is_append)){
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
                open_flags |= O_APPEND;
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
            cout << "后台进程PID：" << child_pid << endl;
        }
    }
}
int main(){
    init_shell();
    while (true) {
        print_prompt();
        string cmd_line;
        if(!getline(cin,cmd_line)){
            cout << endl;
            break;
        }
        size_t start = cmd_line.find_first_not_of(' ');
        size_t end = cmd_line.find_last_not_of(' ');
        if(start==string::npos||end==string::npos){
            continue;
        }
        cmd_line = cmd_line.substr(start, end - start + 1);
        bool is_background = false;
        if(cmd_line.back()=='&'){
            is_background = true;
            end = cmd_line.find_last_not_of('&');
            if(end==string::npos){
                continue;
            }
            cmd_line = cmd_line.substr(0, end + 1);
            end = cmd_line.find_last_not_of(' ');
            if(end==string::npos){
                continue;
            }
            cmd_line = cmd_line.substr(0, end + 1);
        }
        vector<string> cmd_parts = split_command(cmd_line);
        if(!cmd_parts.empty()&&cmd_parts[0]=="exit"{
            cout << "bye bye" << endl;
            break;
        }
        execute_single_command(cmd_parts,is_background);
    }
    return 0;
}
