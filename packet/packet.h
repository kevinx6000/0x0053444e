
// Header
#include "../IP/IP.h"

// Packet class
#ifndef PACKET_H
#define PACKET_H
class Packet{
	public:
		// Constructor
		Packet(void);
		Packet(IP, IP, unsigned int, unsigned int, unsigned int);
		
		// Set function
		void setSrcIP(IP);
		void setDstIP(IP);
		void setSrcPort(unsigned int);
		void setDstPort(unsigned int);
		void setProtocol(unsigned int);
		void setSequence(int);

		// Get function
		IP getSrcIP(void);
		IP getDstIP(void);
		unsigned int getSrcPort(void);
		unsigned int getDstPort(void);
		unsigned int getProtocol(void);
		int getSequence(void);

		// Comparator function
		bool operator<(const Packet&)const;
	private:
		// five-tuple
		IP srcIP;
		IP dstIP;
		unsigned int srcPort;
		unsigned int dstPort;
		unsigned int protocol;
		int seq;
};
#endif
