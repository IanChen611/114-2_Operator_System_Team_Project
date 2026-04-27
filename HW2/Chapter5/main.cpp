#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>

#include "src/fcfs.h"
#include "src/sjf.h"
#include "src/priority.h"


int main(){
    // Setup() => Processes
	// Priority: 1 is the least priority, 4 is the most priority
	std::vector<Process> processes = {
		Process(1,  0, 3, 2),
		Process(2, 10, 8, 1),
		Process(3, 10, 2, 3),
		Process(4, 10, 5, 4)
	};

	// ---First_Come_First_Serve----
	FCFS scheduler_fcfs(processes);
	scheduler_fcfs.startProcess();
	std::cout << "This is Gantt Chart of the First_Come_First_Serve algroitm." << std::endl;
	scheduler_fcfs.showGanttChart();
    // -----------------------------
	std::cout << "==================================================" << std::endl;
	// ---Shortest_Job_First----
	SJF scheduler_sjf(processes);
	scheduler_sjf.startProcess();
	std::cout << "This is Gantt Chart of the Shortest_Job_First algroitm." << std::endl;
	scheduler_sjf.showGanttChart();
    // -----------------------------
	std::cout << "==================================================" << std::endl;

	// ---Priority_Base----
	Priority scheduler_priority(processes);
	scheduler_priority.startProcess();
	std::cout << "This is Gantt Chart of the Priority_Base algroitm." << std::endl;
	scheduler_priority.showGanttChart();
    // -----------------------------	


	return 0;
}
