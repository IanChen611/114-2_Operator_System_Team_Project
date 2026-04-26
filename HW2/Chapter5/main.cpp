#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>

#include "src/fcfs.h"


int main(){
    // ---First_Come_First_Serve----
	std::vector<Process> processes = {
		Process(1, 0, 4, 0),
		Process(2, 1, 3, 0),
		Process(3, 8, 2, 0),
		Process(4, 9, 5, 0)
	};

	FCFS scheduler(processes);
	scheduler.startProcess();
	scheduler.showGanttChart();
    // -----------------------------

	return 0;
}
