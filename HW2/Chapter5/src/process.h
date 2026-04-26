#pragma once

class Process{
protected:
    int pid = 0;
    int arrivalTime = 0;
    int burstTime = 0;
    int remainingTime = 0;
    int priority = 0;
    int completionTime = 0;
    int waitingTime = 0;
    int turnaroundTime = 0;

public:
    Process(int pid, int arrivalTime, int burstTime, int priority);

    int getPid() const;

    void setPid(int value);

    int getArrivalTime() const;

    void setArrivalTime(int value);

    int getBurstTime() const;

    void setBurstTime(int value);

    int getRemainingTime() const;

    void setRemainingTime(int value);

    int getPriority() const;

    void setPriority(int value);

    int getCompletionTime() const;

    void setCompletionTime(int value);

    int getWaitingTime() const;

    void setWaitingTime(int value);

    int getTurnaroundTime() const;

    void setTurnaroundTime(int value);
};