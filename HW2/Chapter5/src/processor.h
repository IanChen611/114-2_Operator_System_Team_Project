#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "process.h"

class Processor{
    
protected:
    std::vector<Process> ps;
    // result: [pid, time] => 各個 pid 執行多久
    /*
    * [1, 2], [2, 2], [1, 2], [-1, 2]
    * => 代表 pid 1 執行 2 秒 => pid 2 執行 2 秒 => pid 1 執行 2 秒 => CPU idle 2 秒
    */
    std::vector<std::pair<int, int>> result;
    int time = 0;

public:
    Processor(std::vector<Process> ps){
        this->ps = ps;
    }

    void showGanttChart(){
        if (result.empty()) {
            std::cout << "No process to display.\n";
            return;
        }

        std::cout << "Gantt Chart\n";
        const int cellWidth = 8;

        for (const auto& segment : result) {
            std::string label;
            if (segment.first == -1) {
                label = "Idle";
            } else {
                label = "P" + std::to_string(segment.first);
            }

            std::cout << '|' << std::left << std::setw(cellWidth - 1) << label;
        }

        std::cout << "|\n";

        int currentTime = 0;
        std::cout << std::right << std::setw(1) << currentTime;
        for (const auto& segment : result) {
            currentTime += segment.second;
            std::cout << std::setw(cellWidth) << currentTime;
        }

        std::cout << '\n';
    }
    virtual void startProcess() = 0;
};