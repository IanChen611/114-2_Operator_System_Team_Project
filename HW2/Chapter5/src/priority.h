#pragma once

#include "processor.h"

class Priority : public Processor{
public:
    Priority(std::vector<Process> ps);
    void startProcess() override;
};