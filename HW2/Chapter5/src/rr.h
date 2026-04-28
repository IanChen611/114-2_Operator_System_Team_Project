#pragma once

#include <queue>
#include <vector>
#include "processor.h"

class RR : public Processor {
private:
    int timeQuantum;

public:
    RR(std::vector<Process> ps, int quantum);
    void startProcess() override;
};
