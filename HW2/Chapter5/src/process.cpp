#include "process.h"

Process::Process(int pid, int arrivalTime, int burstTime, int priority){
	this->pid = pid;
	this->arrivalTime = arrivalTime;
	this->burstTime = burstTime;
	this->remainingTime = burstTime;
	this->priority = priority;
}

int Process::getPid() const {
	return pid;
}

void Process::setPid(int value){
	pid = value;
}

int Process::getArrivalTime() const {
	return arrivalTime;
}

void Process::setArrivalTime(int value){
	arrivalTime = value;
}

int Process::getBurstTime() const {
	return burstTime;
}

void Process::setBurstTime(int value){
	burstTime = value;
}

int Process::getRemainingTime() const {
	return remainingTime;
}

void Process::setRemainingTime(int value){
	remainingTime = value;
}

int Process::getPriority() const {
	return priority;
}

void Process::setPriority(int value){
	priority = value;
}

int Process::getCompletionTime() const {
	return completionTime;
}

void Process::setCompletionTime(int value){
	completionTime = value;
}

int Process::getWaitingTime() const {
	return waitingTime;
}

void Process::setWaitingTime(int value){
	waitingTime = value;
}

int Process::getTurnaroundTime() const {
	return turnaroundTime;
}

void Process::setTurnaroundTime(int value){
	turnaroundTime = value;
}