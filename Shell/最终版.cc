#include <fcntl.h>    
#include <sys/wait.h>
#include <termios.h>  
#include <unistd.h>  
#include <algorithm> 
#include <cstdlib>   
#include <cstring>  
#include <iostream> 
#include <string>   
#include <vector>  
using namespace std;
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define YELLOW "\033[1;33m"  
#define RESET "\033[0m"
#define UP "\033[A"
#define DOWN "\033[B"
vector<string> history_cmds;  
int history_pos = -1;        
string current_input;        
string cwd, prev_cwd;
const int MAX_CMD = 100;
const int MAX_PIPE = 50;
void clean_space(string &s){
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ")+1);
    size_t pos;
    while((pos=s.find("  "))!=string::npos){
        s.erase(pos, 1);
    }
}
vector<string> split_str(const string&s,char zhiding){
    vector<string> res;
    string temp;
    for(char c:s){
        if(c==zhiding){
            clean_space(temp);
            if(!temp.empty()){
                res.push_back(temp);
            }
            temp.clear();
        }else{
            temp += c;
        }
    }
    clean_space(temp);
    if(!temp.empty()){
        res.push_back(temp);
    }
    return res;
}
vector<char*> parse_cmd(const string&cmd,string&infile,string&outfile,bool&append,bool&err_out){
    vector<char*> argv;
    vector<string> parts = split_str(cmd, ' ');
    infile = outfile = "";
    append = err_out = false;
    for (int i = 0; i < parts.size();i++){
        if(parts[i]=="<"&&i+1<parts.size()){
            infile = parts[i + 1];
            i++;
        }else if(parts[i]==">"&&i+1<parts.size()){
            outfile = parts[i + 1];
            i++;
        }else if(parts[i]==">>"&&i+1<parts.size()){
            outfile = parts[i + 1];
            i++;
            append = true;
        }else if(parts[i]=="2>"&&i+1<parts.size()){
            outfile = parts[i + 1];
            i++;
            err_out = true;
        }else{
            char* arg = new char[parts[i].size() + 1];
            strcpy(arg, parts[i].c_str());
            argv.push_back(arg);
        }
    }
    argv.push_back(nullptr);
    return argv; 
}
void do_redirect(const string&infile,const string&outfile,bool append,bool err_out){
    if(!infile.empty()){
        int fd = open(infile.c_str(), O_RDONLY);
        if(fd>0){
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
    }
    if(!outfile.empty()){
        int flag = O_WRONLY | O_CREAT;
        flag |= append ? O_APPEND : O_TRUNC;
        int fd = open(outfile.c_str(), flag, 0644);
        if(fd>0){
            dup2(fd, err_out ? STDERR_FILENO : STDOUT_FILENO);
            close(fd);
        }
    }
}
void do_cd(const string& cmd) {
    vector<string> parts = split_str(cmd, ' ');
    string home_dir = getenv("HOME") ? getenv("HOME") : "/";
    string dir = parts.size() < 2 ? home_dir : parts[1];
    if (dir == "~") {
        dir = home_dir;
    }
    else if (dir == "-") {
        if (prev_cwd.empty()) {
            cout << RED << "cd: 没有上一次的工作目录" << RESET << endl;
            return;
        }
        dir = prev_cwd;
        cout << CYAN << dir << RESET << endl;
    }
    if (chdir(dir.c_str()) == -1) {
        cout << RED << "cd: 无法切换到目录 '" << dir << RESET << endl;
        return;
    }
    char buf[1024];
    prev_cwd = cwd;
    cwd = getcwd(buf, sizeof(buf));
}
void show_prompt(){
        char buf[1024];
        cwd = getcwd(buf, sizeof(buf));  
        char* user = getenv("USER"); 
        char hostname[256];
        gethostname(hostname, sizeof(hostname)); 
        string show_dir;
        string home_dir = getenv("HOME") ? getenv("HOME") : "/";
        if (cwd == home_dir) {
            show_dir = "~";
        } else {
            show_dir = cwd.substr(cwd.find_last_of('/') + 1);
        }
        cout  << YELLOW << user << "@" << hostname
             << RESET                             
        << " " <<CYAN << show_dir << RESET        
         << GREEN << " → " << RESET;  
        cout.flush();
}
void run_command(
    const vector<string>& cmds, bool bg) { 
    int pipefd[MAX_PIPE][2];  
    int n = cmds.size();
    pid_t pid[MAX_CMD];
    for (int i = 0; i < n - 1;i++){
        pipe(pipefd[i]);
    }
    for (int i = 0; i < n;i++){
        pid[i] =fork();  
        if(pid[i]==0){
            string infile, outfile;
            bool append = false, err_out = false;
            vector<char*> argv =
                parse_cmd(cmds[i], infile, outfile, append, err_out);
            if (i > 0){
                dup2(pipefd[i - 1][0], STDIN_FILENO);
            }
            if (i < n - 1){
                dup2(pipefd[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < n - 1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }
            do_redirect(infile, outfile, append, err_out);
            if (bg) {
                int FD = open("/dev/null", O_WRONLY);
                if (FD > 0) {
                    dup2(FD, STDOUT_FILENO); 
                    dup2(FD, STDERR_FILENO);  
                    close(FD);
                }
            }
            if (execvp(argv[0], argv.data()) == -1) {
                cout << RED<<"未找到命令: " << argv[0]<<RESET << endl;
                exit(1); 
        }
    }
    }
    for (int i = 0; i < n - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
    if (!bg) {
        for (int i = 0; i < n; i++)
            waitpid(pid[i], nullptr, 0);
    } else {
        cout << YELLOW << "后台运行共启动 " << n << " 个子进程"
             << RESET << endl;
    }
}
 void add_history_cmd(const string& cmd) {
     string temp = cmd;
     clean_space(temp);  
     if (temp.empty())
         return;
     if (!history_cmds.empty() && history_cmds.back() == temp)
         return;
     history_cmds.push_back(temp);
 }
 string read_cmd() {
     string cmd;
     char ch;
     struct termios old, newt;
     tcgetattr(STDIN_FILENO, &old);
     newt = old;
     newt.c_lflag &= ~(ICANON | ECHO);
     tcsetattr(STDIN_FILENO, TCSANOW, &newt);

     while (read(STDIN_FILENO, &ch, 1) == 1) {
         if (ch == '\n') {
             tcsetattr(STDIN_FILENO, TCSANOW, &old);
             cout << endl;
             return cmd;
         }
         else if (ch == 127) {
             if (!cmd.empty()) {
                 cmd.pop_back();
                 cout << "\b \b" << flush;
             }
         }
         else if (ch == '\033') {
             char seq[3] = {ch};
             read(STDIN_FILENO, &seq[1], 2);  
             string fangxiang(seq, 3);
             if (fangxiang == UP && !history_cmds.empty()) {
                 history_pos = (history_pos == -1) ? history_cmds.size() - 1
                                                   : history_pos - 1;
                 if (history_pos < 0)
                     history_pos = 0;
                 cout << "\r \r";
                 show_prompt();
                 cout << history_cmds[history_pos] << flush;
                 cmd = history_cmds[history_pos];
             }
             else if (fangxiang == DOWN && !history_cmds.empty() &&
                      history_pos != -1) {
                 if (history_pos < history_cmds.size() - 1)
                     history_pos++;
                 else {
                     history_pos = -1;
                     cmd = "";
                 }
                 cout << "\r \r";
                 show_prompt();
                 if (history_pos != -1)
                     cout << history_cmds[history_pos];
                 cout << flush;
                 cmd = history_pos == -1 ? "" : history_cmds[history_pos];
             }
         }
         else {
             cmd += ch;
             cout << ch << flush;
         }
     }
     tcsetattr(STDIN_FILENO, TCSANOW, &old);
     return cmd;
 }
int main() {
    system("stty -echoctl"); 
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    string cmd;
    char buf[1024];
    cwd = prev_cwd = getcwd(buf, sizeof(buf));
    while (true) {
        show_prompt();
        cmd = read_cmd();  
        clean_space(cmd);
        if (cmd.empty())
            continue;
        add_history_cmd(cmd);  
        bool bg_run = false;
        if (cmd.back() == '&') {
            bg_run = true;
            cmd.pop_back();
            clean_space(cmd);
        }
        if (cmd == "exit") {
            cout << GREEN << "退出Shell，再见" << RESET << endl;
            break;
        }
        if (cmd.substr(0, 2) == "cd") {
            do_cd(cmd);
            continue;
        }
        vector<string> pipe_cmds = split_str(cmd, '|');
        run_command(pipe_cmds, bg_run);
    }
    return 0;
}