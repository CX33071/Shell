#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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
string cwd, prev_cwd;
const int MAX_CMD = 100;
const int MAX_PIPE = 50;
struct bg {
    int id;
    pid_t pid;
    string cmd;
    bool is_running;
};
vector<bg> bgs;
void clean_space(string& s) {
    int start = s.find_first_not_of(" ");
    int end = s.find_last_not_of(" ");
    if (start == -1) {
        s = "";
    } else {
        s = s.substr(start, end - start + 1);
    }
}
vector<string> split_str(const string& s, char zhiding) {
    vector<string> res;
    string temp;
    for (char c : s) {
        if (c == zhiding) {
            clean_space(temp);
            if (!temp.empty()) {
                res.push_back(temp);
            }
            temp.clear();
        } else {
            temp += c;
        }
    }
    clean_space(temp);
    if (!temp.empty()) {
        res.push_back(temp);
    }
    return res;
}
string show_prompt() {
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
    string prompt = YELLOW + string(user) + "@" + string(hostname) + RESET +
                    " " + CYAN + show_dir + RESET + GREEN + " → " + RESET;
    return prompt;
}
vector<char*> parse_cmd(const string& cmd,
                        string& infile,
                        string& outfile,
                        bool& append,
                        bool& err_out) {
    vector<char*> argv;
    vector<string> parts = split_str(cmd, ' ');
    infile = outfile = "";
    append = err_out = false;
    for (int i = 0; i < parts.size(); i++) {
        if (parts[i] == "<" && i + 1 < parts.size()) {
            infile = parts[i + 1];
            i++;
        } else if (parts[i] == ">" && i + 1 < parts.size()) {
            outfile = parts[i + 1];
            i++;
        } else if (parts[i] == ">>" && i + 1 < parts.size()) {
            outfile = parts[i + 1];
            i++;
            append = true;
        } else if (parts[i] == "2>" && i + 1 < parts.size()) {
            outfile = parts[i + 1];
            i++;
            err_out = true;
        } else {
            char* arg = new char[parts[i].size() + 1];
            strcpy(arg, parts[i].c_str());
            argv.push_back(arg);
        }
    }
    argv.push_back(nullptr);
    return argv;
}
void do_redirect(const string& infile,
                 const string& outfile,
                 bool append,
                 bool err_out) {
    if (!infile.empty()) {
        int fd = open(infile.c_str(), O_RDONLY);
        if (fd > 0) {
            dup2(fd, STDIN_FILENO);
            close(fd);
        } else {
            cout << RED << "重定向错误: " << infile << " 不存在" << RESET
                 << endl;
            exit(1);
        }
    }
    if (!outfile.empty()) {
        int flag = O_WRONLY | O_CREAT;
        flag |= append ? O_APPEND : O_TRUNC;
        int fd = open(outfile.c_str(), flag, 0644);
        if (fd > 0) {
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
    } else if (dir == "-") {
        if (prev_cwd.empty()) {
            cout << RED << "cd: 没有上一次的工作目录" << RESET << endl;
            return;
        }
        dir = prev_cwd;
        cout << CYAN << dir << RESET << endl;
    }
    if (chdir(dir.c_str()) == -1) {
        cout << RED << "cd: 无法切换到目录 " << dir << RESET << endl;
        return;
    }
    char buf[1024];
    prev_cwd = cwd;
    cwd = getcwd(buf, sizeof(buf));
}
int get_id() {
    int id = 1;
    while (true) {
        bool is_use = false;
        for (auto& bg : bgs) {
            if (bg.id == id && bg.is_running) {
                is_use = true;
                break;
            }
        }
        if (!is_use) {
            break;
        }
        id++;
    }
    return id;
}
void run_command(const vector<string>& cmds, bool bg, string cmd2) {
    int pipefd[MAX_PIPE][2];
    int n = cmds.size();
    if (n == 0) {
        return;
    }
    pid_t pid[MAX_CMD];
    for (int i = 0; i < n - 1; i++) {
        pipe(pipefd[i]);
    }
    for (int i = 0; i < n; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            string infile, outfile;
            bool append = false, err_out = false;
            vector<char*> argv =
                parse_cmd(cmds[i], infile, outfile, append, err_out);
            if (i > 0) {
                dup2(pipefd[i - 1][0], STDIN_FILENO);
            }
            if (i < n - 1) {
                dup2(pipefd[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < n - 1; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }
            do_redirect(infile, outfile, append, err_out);
            if (execvp(argv[0], argv.data()) == -1) {
                cout << RED << "未找到命令: " << argv[0] << RESET << endl;
                exit(1);
            }
        }
    }
    for (int i = 0; i < n - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
    if (!bg) {
        for (int i = 0; i < n; i++) {
            waitpid(pid[i], nullptr, 0);
        }
    } else {
        cout << "\n";
        int id = get_id();
        cout << "[" << id << "]   " << pid[0] << endl;
        bgs.push_back({id, pid[0], cmd2, true});
    }
}
string lscolor(const string& cmd) {
    string new_cmd = cmd;
    size_t lspos = new_cmd.find("ls");
    if (lspos != string::npos && (lspos == 0 || new_cmd[lspos - 1] == ' ') &&
        (lspos + 2 >= new_cmd.size() || new_cmd[lspos + 2] == ' ')) {
        new_cmd.insert(lspos + 2, " --color=tty");
    }
    return new_cmd;
}
void sign(int sign) {
    (void)sign;
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (auto& bg : bgs) {
            if (bg.pid == pid && bg.is_running) {
                string STATUS = WIFEXITED(status) ? "Done" : "terminated";
                cout << "\n[" << bg.id << "]  +" << "  " << STATUS << "   "
                     << pid << "    " << bg.cmd << endl;
                bg.is_running = false;
                rl_on_new_line();
                rl_redisplay();
                break;
            }
        }
    }
}
int main() {
    system("stty -echoctl");
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, sign);
    using_history();
    string cmd;
    char buf[1024];
    cwd = prev_cwd = getcwd(buf, sizeof(buf));
    while (true) {
        char* input = readline(show_prompt().c_str());
        if (input == nullptr) {
            cout << endl;
            break;
        }
        cmd = input;
        free(input);
        clean_space(cmd);
        if (cmd.empty()) {
            continue;
        }
        add_history(cmd.c_str());
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
        string cmd2 = lscolor(cmd);
        vector<string> pipe_cmds = split_str(cmd2, '|');
        run_command(pipe_cmds, bg_run, cmd2);
    }
    bgs.clear();
    return 0;
}
