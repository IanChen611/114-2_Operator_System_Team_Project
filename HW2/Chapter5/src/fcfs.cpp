#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "fcfs.h"

FCFS::FCFS(std::vector<Process> ps) : Processor(ps) {

}

void FCFS::startProcess(){
	result.clear();
	time = 0;

	std::stable_sort(ps.begin(), ps.end(), [](const Process& left, const Process& right) {
		if (left.getArrivalTime() != right.getArrivalTime()) {
			return left.getArrivalTime() < right.getArrivalTime();
		}

		return left.getPid() < right.getPid();
	});

	for (auto& process : ps) {
		if (time < process.getArrivalTime()) {
			result.push_back({-1, process.getArrivalTime() - time});
			time = process.getArrivalTime();
		}

		const int completionTime = time + process.getBurstTime();
		const int turnaroundTime = completionTime - process.getArrivalTime();
		const int waitingTime = turnaroundTime - process.getBurstTime();

		process.setRemainingTime(0);
		process.setCompletionTime(completionTime);
		process.setTurnaroundTime(turnaroundTime);
		process.setWaitingTime(waitingTime);

		result.push_back({process.getPid(), process.getBurstTime()});
		time = completionTime;
	}
}