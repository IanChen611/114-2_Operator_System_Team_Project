#include <algorithm>
#include <map>
#include <queue>
#include "priority_rr.h"

PriorityRR::PriorityRR(std::vector<Process> ps, int quantum) : Processor(ps), timeQuantum(quantum) {}

void PriorityRR::startProcess() {
    result.clear();
    time = 0;
    
    for (auto& p : ps) {
        p.setRemainingTime(p.getBurstTime());
    }

    int n = ps.size();
    int completed = 0;

    // 用於追蹤每個優先級的 Ready Queue
    std::map<int, std::queue<Process*>> readyQueues;
    std::vector<bool> inQueue(n, false);

    while (completed < n) {
        // 將所有新抵達的程序加入對應優先級的隊列
        bool anyNewArrival = false;
        for (int i = 0; i < n; i++) {
            if (!inQueue[i] && ps[i].getArrivalTime() <= time) {
                readyQueues[ps[i].getPriority()].push(&ps[i]);
                inQueue[i] = true;
                anyNewArrival = true;
            }
        }

        // 尋找目前有程序的最優先隊列 (數值大優先權高)
        int highestPriority = -1;
        for (auto it = readyQueues.rbegin(); it != readyQueues.rend(); ++it) {
            if (!it->second.empty()) {
                highestPriority = it->first;
                break;
            }
        }

        if (highestPriority == -1) {
            // CPU Idle
            int nextArrival = 2147483647;
            bool found = false;
            for (int i = 0; i < n; i++) {
                if (!inQueue[i]) {
                    nextArrival = std::min(nextArrival, ps[i].getArrivalTime());
                    found = true;
                }
            }
            if (found) {
                result.push_back({-1, nextArrival - time});
                time = nextArrival;
            } else {
                break;
            }
            continue;
        }

        Process* current = readyQueues[highestPriority].front();
        readyQueues[highestPriority].pop();

        int runTime = std::min(current->getRemainingTime(), timeQuantum);
        
        // 檢查是否有更高優先權的程序會在 runTime 期間抵達 (搶佔)
        int actualRunTime = runTime;
        for (int i = 0; i < n; i++) {
            if (!inQueue[i] && ps[i].getArrivalTime() < time + runTime) {
                if (ps[i].getPriority() > highestPriority) {
                    actualRunTime = std::max(0, ps[i].getArrivalTime() - time);
                    break;
                }
            }
        }

        if (actualRunTime > 0) {
            result.push_back({current->getPid(), actualRunTime});
            current->setRemainingTime(current->getRemainingTime() - actualRunTime);
            time += actualRunTime;
        } else if (actualRunTime == 0 && current->getRemainingTime() > 0) {
            // 被立即搶佔，不執行
        }

        // 更新隊列：加入在執行期間抵達的所有程序
        for (int i = 0; i < n; i++) {
            if (!inQueue[i] && ps[i].getArrivalTime() <= time) {
                readyQueues[ps[i].getPriority()].push(&ps[i]);
                inQueue[i] = true;
            }
        }

        if (current->getRemainingTime() > 0) {
            readyQueues[current->getPriority()].push(current);
        } else {
            if (actualRunTime > 0 || current->getBurstTime() == 0) {
                current->setCompletionTime(time);
                current->setTurnaroundTime(time - current->getArrivalTime());
                current->setWaitingTime(current->getTurnaroundTime() - current->getBurstTime());
                completed++;
            } else {
                 readyQueues[current->getPriority()].push(current);
            }
        }
    }
}
