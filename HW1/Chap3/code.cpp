/**
 * 112820025 陳翊揚 須完成的實作：
 *   1. 主迴圈 (Main Loop) 
 *   2. 基本指令執行   fork() + exec()
 *   3. 歷史功能 (History Feature)
 * 
 * 112820003 辛政隆 須完成的實作：
 *   1. 加入 Redirection 和 Pipe
 */

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80
#define HISTORY_SIZE 10

// ==========================================
// 解析後的指令結構
// ==========================================
class Command{
public:
    std::vector<std::string> args;
    bool run_background = false;

    // 會用到的欄位
    bool has_redirect_in  = false;
    bool has_redirect_out = false;
    bool has_pipe         = false;
    std::string redirect_in_file;
    std::string redirect_out_file;
    std::vector<std::string> pipe_args;
};

// ==========================================
// 歷史紀錄
// ==========================================

std::array<std::string, HISTORY_SIZE> history;
int history_count = 0;



// 將 `指令 line` 存入 history 陣列
void add_history(const std::string &line) {
    // 因為 history 最多存 HISTORY_SIZE 個，需循環使用 history
    history[history_count % HISTORY_SIZE] = line;
    history_count++;
}

// 印出所有歷史紀錄
void print_history() {
    // 格式為 "1 ls\n2 ls -la\n..."
    int start = (history_count > HISTORY_SIZE) ? 
                 history_count - HISTORY_SIZE : 0;
    
    for (int i = start; i < history_count; i++) {
        std::cout << i + 1 << " " 
                  << history[i % HISTORY_SIZE] << "\n";
    }
}

// ==========================================
// 解析指令
// ==========================================
Command parse_command(const std::string &line) {
    Command cmd;
    std::istringstream iss(line);
    std::string token;

    // 用 while 將 line 一個一個指令取出放進 token
    // token 
    while (iss >> token) {
        if (token == "&") {
            // 設定背景執行
            cmd.run_background = true;
        }
        else if (token == "<") {
            // TODO
            cmd.has_redirect_in = true;
            iss >> cmd.redirect_in_file;
        }
        else if (token == ">") {
            // TODO
            cmd.has_redirect_out = true;
            iss >> cmd.redirect_out_file;
        }
        else if (token == "|") {
            // TODO
            cmd.has_pipe = true;
            while (iss >> token)
                cmd.pipe_args.push_back(token);
        }
        else {
            // 將 token 加入 cmd.args
            cmd.args.push_back(token);
        }
    }

    return cmd;
}

// ==========================================
// 把 vector<string> 轉成 execvp 需要的 char*[]
// ==========================================

// 把 args 裡的每個字串轉成 char*
std::vector<char*> to_argv(const std::vector<std::string> &args) {
    std::vector<char*> result;
    for(const auto &arg : args){
        result.push_back(const_cast<char*>(arg.c_str()));
    }
    result.push_back(nullptr);
    return result;
}

// ==========================================
// 執行指令
// ==========================================
void execute(const Command &cmd) {
    // 1: 呼叫 fork()
    pid_t pid = fork();

    // 2: 處理 fork 失敗的情況
    if (pid < 0) {
        std::cerr << "Fork 失敗!\n" ;
        exit(1);
    } 

    // 3: 子行程
    else if(pid == 0){
        // - 呼叫 to_argv() 取得 argv
        std::vector<char*> argv = to_argv(cmd.args);
        // - 呼叫 execvp() 執行指令
        execvp(argv[0], argv.data());
        
        // - 處理 execvp 失敗的情況
        // 若 execvp 沒有執行就不會提前 exit()
        std::cerr << "指令不存在: " << cmd.args[0] << "\n";
        exit(1);
    }
    
    // 4: 父行程
    //   - 若非背景執行，呼叫 wait()
    //   - 若是背景執行，印出 PID
    else{
        if (!cmd.run_background)
            wait(NULL);
        else
            std::cout << "[背景執行] PID = " << pid << "\n";
    }
}

// ==========================================
// 主迴圈
// ==========================================
int main() {
    std::string line;

    while (true) {
        // 1: 印出提示字元 "osh> "
        std::cout << "osh>";
        std::cout.flush(); // 怕 getline 讀到

        // 2: 用 getline() 讀取輸入，若讀取失敗則 break
        if (!std::getline(std::cin, line)) break;

        // 3: 若輸入為空則 continue
        if(line.empty()) continue;

        // 4: 處理 !! 指令
        //   - 若 history 為空，印出 "No commands in history."
        //   - 否則取出上一筆指令，印出並執行
        if(line == "!!"){
            if(history_count == 0){
                std::cout << "No commands in history." << std::endl;
                continue;
            }
            line = history[(history_count - 1) % HISTORY_SIZE];
            std::cout << line << "\n";
        } 
        
        // 5: 將指令加入歷史紀錄
        add_history(line);

        // 6: 呼叫 parse_command() 解析指令
        Command cmd = parse_command(line);
        if(cmd.args.empty()) continue;

        // 7: 處理內建指令
        //   - "exit" → 結束迴圈
        //   - "history" → 印出歷史紀錄
        if(cmd.args[0] == "exit") break;
        if(cmd.args[0] == "history") {
            print_history();
            continue;
        }

        // 8: 呼叫 execute() 執行指令
        execute(cmd);
    }

    return 0;
}