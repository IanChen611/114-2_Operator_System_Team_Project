#pragma once

#include <vector>
#include <map>
#include <queue>
#include "processor.h"

class PriorityRR : public Processor {
private:
    int timeQuantum;

public:
    PriorityRR(std::vector<Process> ps, int quantum);
    void startProcess() override;
};
