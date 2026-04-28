#include <algorithm>
#include <queue>
#include "rr.h"

RR::RR(std::vector<Process> ps, int quantum) : Processor(ps), timeQuantum(quantum) {}

void RR::startProcess() {
    result.clear();
    time = 0;
    
    // 初始化剩餘時間
    for (auto& p : ps) {
        p.setRemainingTime(p.getBurstTime());
    }

    // 按抵達時間排序以方便處理進入 Ready Queue
    std::vector<Process*> remainingPs;
    for (auto& p : ps) {
        remainingPs.push_back(&p);
    }
    std::sort(remainingPs.begin(), remainingPs.end(), [](Process* a, Process* b) {
        return a->getArrivalTime() < b->getArrivalTime();
    });

    std::queue<Process*> readyQueue;
    int processIdx = 0;
    int completed = 0;
    int n = ps.size();

    while (completed < n) {
        // 將所有已抵達的程序加入 Ready Queue
        while (processIdx < n && remainingPs[processIdx]->getArrivalTime() <= time) {
            readyQueue.push(remainingPs[processIdx]);
            processIdx++;
        }

        if (readyQueue.empty()) {
            if (processIdx < n) {
                // CPU Idle
                int nextArrival = remainingPs[processIdx]->getArrivalTime();
                result.push_back({-1, nextArrival - time});
                time = nextArrival;
                continue;
            } else {
                break;
            }
        }

        Process* current = readyQueue.front();
        readyQueue.pop();

        int runTime = std::min(current->getRemainingTime(), timeQuantum);
        result.push_back({current->getPid(), runTime});
        
        current->setRemainingTime(current->getRemainingTime() - runTime);
        time += runTime;

        // 在執行期間抵達的程序也必須加入隊列
        while (processIdx < n && remainingPs[processIdx]->getArrivalTime() <= time) {
            readyQueue.push(remainingPs[processIdx]);
            processIdx++;
        }

        if (current->getRemainingTime() > 0) {
            readyQueue.push(current);
        } else {
            current->setCompletionTime(time);
            current->setTurnaroundTime(time - current->getArrivalTime());
            current->setWaitingTime(current->getTurnaroundTime() - current->getBurstTime());
            completed++;
        }
    }
}
