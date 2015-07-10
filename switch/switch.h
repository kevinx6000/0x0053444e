
// Headers
#include "../basic_lib.h"
#include "../node/node.h"
#include "../entry/entry.h"
#include "../event/event.h"
#include "../packet/packet.h"

// Switch class
#ifndef SWITCH_H
#define SWITCH_H
class Switch: public Node{
	public:
		Switch(int);
		Event forward(double,Packet);
		vector<Entry>TCAM;
		vector<Packet>que;
		map<Packet,bool>isSetup;
		double posXY[2];
};
#endif
