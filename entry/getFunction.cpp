
// Headers
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../entry/entry.h"
#include "../IP/IP.h"

int Entry::getOutputPort(void){
	return outputPort;
}
int Entry::getPriority(void){
	return priority;
}
int Entry::getSID(void){
	return this->sid;
}
double Entry::getRate(void){
	return this->rate;
}
