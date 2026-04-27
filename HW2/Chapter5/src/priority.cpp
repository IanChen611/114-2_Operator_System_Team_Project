#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "priority.h"

Priority::Priority(std::vector<Process> ps) : Processor(ps) {

}

void Priority::startProcess(){
	result.clear();
	time = 0;

	std::vector<bool> done(ps.size(), false);
	int completed = 0;

	while (completed < (int)ps.size()) {
		int best = -1;
		for (int i = 0; i < (int)ps.size(); i++) {
			if (done[i] || ps[i].getArrivalTime() > time) continue;
			if (best == -1
				|| ps[i].getPriority() > ps[best].getPriority()
				|| (ps[i].getPriority() == ps[best].getPriority()
					&& ps[i].getArrivalTime() < ps[best].getArrivalTime())) {
				best = i;
			}
		}

		if (best == -1) {
			int nextArrival = std::numeric_limits<int>::max();
			for (int i = 0; i < (int)ps.size(); i++) {
				if (!done[i]) nextArrival = std::min(nextArrival, ps[i].getArrivalTime());
			}
			result.push_back({-1, nextArrival - time});
			time = nextArrival;
		} else {
			auto& p = ps[best];
			const int completionTime = time + p.getBurstTime();
			p.setCompletionTime(completionTime);
			p.setTurnaroundTime(completionTime - p.getArrivalTime());
			p.setWaitingTime(p.getTurnaroundTime() - p.getBurstTime());
			p.setRemainingTime(0);
			result.push_back({p.getPid(), p.getBurstTime()});
			time = completionTime;
			done[best] = true;
			completed++;
		}
	}
}