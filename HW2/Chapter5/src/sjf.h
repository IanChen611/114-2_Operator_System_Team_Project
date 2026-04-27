#pragma once

#include "processor.h"

class SJF : public Processor{
public:
    SJF(std::vector<Process> ps);
    void startProcess() override;
};