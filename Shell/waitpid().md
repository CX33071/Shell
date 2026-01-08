waitpid()

1.为什么需要waitpid？

子进程执行完后如果没人 “收尸”，会变成**僵尸进程**（占用系统资源）；

前台命令（如`ls -l`）需要 Shell 等待它执行完再显示提示符；

后台命令（如`sleep 10 &`）需要 Shell 不等待，直接返回提示符

2.函数原型

```c++
pid_t waitpid(pid_t pid, int *wstatus, int options);
```

| 参数      | 类型    | 核心作用                                                     |                                                              |
| --------- | ------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `pid`     | `pid_t` | 指定要等待的子进程（核心！）                                 | - `>0`：等待 PID 为该值的子进程（前台命令）- `-1`：等待任意子进程（回收僵尸进程）- `0`：等待同进程组的子进程- |
| `wstatus` | `int*`  | 存储子进程的退出状态（退出码、终止原因）                     | - `nullptr`：不关心退出状态（简单场景）- `&status`：获取退出状态（需要判断子进程是正常退出还是被信号终止） |
| `options` | `int`   | 等待方式（阻塞 / 非阻塞）                                    | - `0`：阻塞等待（前台命令，默认）- `WNOHANG`：非阻塞等待（后台命令 / 回收僵尸进程） |
| 返回值    | `pid_t` | 成功：返回被等待的子进程 PID；失败：返回`-1`；非阻塞时无子进程结束返回`0` |                                                              |

3.用法：

（1）阻塞等待前台子进程

```c++
// 创建子进程执行命令
pid_t child_pid = fork();
if (child_pid == 0) {
    // 子进程：执行命令
    execvp(cmd_argv[0], cmd_argv.data());
    exit(EXIT_FAILURE);
} else if (child_pid > 0) {
    // 父进程：阻塞等待该子进程结束（前台命令）
    int status;
    pid_t result = waitpid(child_pid, &status, 0); // options=0 → 阻塞
    if (result == -1) {
        cerr << "等待子进程失败：" << strerror(errno) << endl;
    }
    // 可选：解析子进程退出状态
    if (WIFEXITED(status)) {
        // 正常退出，获取退出码
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            cerr << "命令执行失败，退出码：" << exit_code << endl;
        }
    } else if (WIFSIGNALED(status)) {
        // 被信号终止（如Ctrl+C），获取信号码
        int sig = WTERMSIG(status);
        cerr << "命令被信号" << sig << "终止" << endl;
    }
}

```

**核心效果**：Shell 会卡住，直到`ls -l`执行完，才会再次显示提示符；

**关键参数**：`pid=child_pid`（只等这个子进程）、`options=0`（阻塞）

（2）非阻塞回收后台僵尸进程

```c++
// 父进程：不阻塞，直接返回提示符，同时定期回收僵尸进程
pid_t child_pid = fork();
if (child_pid > 0) {
    cout << "后台进程PID：" << child_pid << endl;
    // 非阻塞等待（WNOHANG），立即返回，不阻塞Shell主循环
    int status;
    pid_t result = waitpid(child_pid, &status, WNOHANG);
    if (result == 0) {
        // 子进程还在运行，正常返回，Shell继续显示提示符
    } else if (result > 0) {
        // 子进程已结束，直接回收（极少发生，因为后台命令刚创建）
    }
}

// Shell主循环中，定期回收所有已结束的子进程（核心！）
void reap_zombies() {
    int status;
    pid_t pid;
    // 循环非阻塞等待所有已结束的子进程，直到没有为止
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        cout << "回收僵尸进程：" << pid << endl;
    }
}

```

**核心效果**：执行`sleep 10 &`后，Shell 立即显示提示符，同时后台子进程运行，结束后会被`waitpid(-1, ..., WNOHANG)`回收；

**关键参数**：`pid=-1`（等待任意子进程）、`options=WNOHANG`（非阻塞）

(3)不关心子进程退出状态

```c++
// 父进程：阻塞等待，不关心退出状态
waitpid(child_pid, nullptr, 0);
```

4.解析子进程退出状态

`wstatus` 存储了子进程的退出细节，需要用系统宏来解析（不能直接读值），常用宏如下：

| 宏                    | 作用                                                         | 你的 Shell 场景用法      |
| --------------------- | ------------------------------------------------------------ | ------------------------ |
| `WIFEXITED(status)`   | 判断子进程是否**正常退出**（如`exit(0)`/`return 0`）         | 正常退出则判断退出码     |
| `WEXITSTATUS(status)` | 获取正常退出的**退出码**（0 = 成功，非 0 = 失败）            | 打印命令执行失败的退出码 |
| `WIFSIGNALED(status)` | 判断子进程是否**被信号终止**（如 Ctrl+C=SIGINT，kill=SIGKILL） | 提示 “命令被信号终止”    |
| `WTERMSIG(status)`    | 获取终止子进程的**信号码**（如 2=SIGINT，9=SIGKILL）         | 显示被哪个信号终止       |

`wait()` 只能 “阻塞等待任意子进程”，无法实现后台命令和精准控制