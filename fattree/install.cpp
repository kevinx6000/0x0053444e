
// Headers
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../node/node.h"
#include "../switch/core.h"
#include "../switch/aggregate.h"
#include "../switch/edge.h"
#include "../host/host.h"
#include "../fattree/fattree.h"
#include "../event/event.h"
#include "../event/eventType.h"

// Install event and forward
void Fattree::install(Event evt){

	// Variables
	int ts = evt.getTimeStamp();
	int nid = evt.getID();
	Entry ent = evt.getEntry();
	Packet pkt = evt.getPacket();
	Event ret;

	// Remove the entries of the same flow ID
	for(int i = 0; i < sw[nid]->TCAM.size(); i++)
		if(sw[nid]->TCAM[i].isMatch(pkt)){
			sw[nid]->TCAM.erase(sw[nid]->TCAM.begin() + i);
			i--;
		}

	// Remove if full
	if(sw[nid]->TCAM.size() >= maxEntry)
		sw[nid]->TCAM.erase(sw[nid]->TCAM.begin());

	// Install at the tail
	sw[nid]->TCAM.push_back(ent);
}
