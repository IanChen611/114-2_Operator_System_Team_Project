#pragma once

#include "processor.h"

class FCFS : public Processor{
public:
    FCFS(std::vector<Process> ps);
    void startProcess() override;
};