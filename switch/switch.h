
// Headers
#include "../basic_lib.h"
#include "../node/node.h"
#include "../entry/entry.h"
#include "../event/event.h"
#include "../packet/packet.h"
#include "../mylist/mylist.h"

// Switch class
#ifndef SWITCH_H
#define SWITCH_H
class Switch: public Node{
	public:
		Switch(int);
		Event forward(double,Packet);
		vector<Packet>que;
		map<Packet,bool>isSetup;
		double posXY[2];
		vector<EDGE>wlink;
		vector< vector<int> >iList;
		double APrate;
		MyLL TCAM;
		map<Packet,LLNODE*>TCAMmap;
};
#endif
