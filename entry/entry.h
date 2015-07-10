
// Headers
#include "../basic_lib.h"
#include "../packet/packet.h"

// Entry class
#ifndef ENTRY_H
#define ENTRY_H
class Entry{
	public:
		Entry(int s0 = -1, int s1 = -1, int s2 = -1, int s3 = -1,
			int d0 = -1, int d1 = -1, int d2 = -1, int d3 = -1, 
			int out = 0, int pri = 0);
		bool isMatch(Packet);
		void setSrcMask(int,int,int,int);
		void setDstMask(int,int,int,int);
		void setOutputPort(int);
		void setPriority(int);
		void setSID(int);
		void setRate(double);
		int getOutputPort(void);
		int getPriority(void);
		int getSID(void);
		double getRate(void);
	private:
		int sid;
		int srcMask[4];
		int dstMask[4];
		int outputPort;
		int priority;
		double rate;
};
#endif
