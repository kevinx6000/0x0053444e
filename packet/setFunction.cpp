// Packet class
#include "../packet/packet.h"
#include "../IP/IP.h"

// Packet class definition
void Packet::setSrcIP(IP srcIP){
	this->srcIP = srcIP;
}
void Packet::setDstIP(IP dstIP){
	this->dstIP = dstIP;
}
void Packet::setSrcPort(unsigned int srcPort){
	this->srcPort = srcPort;
}
void Packet::setDstPort(unsigned int dstPort){
	this->dstPort = dstPort;
}
void Packet::setProtocol(unsigned int protocol){
	this->protocol = protocol;
}
void Packet::setSequence(int ss){
	this->seq = ss;
}
void Packet::setNumPkt(unsigned int numPkt){
	this->numPkt = numPkt;
}
void Packet::setDataRate(double dataRate){
	this->dataRate = dataRate;
}
