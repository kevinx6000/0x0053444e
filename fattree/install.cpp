
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
	Packet tmpPkt;
	Event ret;

	/* Maybe here we need to remove expired entries??? */

	// If TCAM is full
	if(sw[nid]->TCAM.size() >= maxEntry){

		// Remove the front most entry
		tmpPkt = sw[nid]->TCAM.front().getSample();
		sw[nid]->TCAMmap.erase(tmpPkt);
		sw[nid]->TCAM.pop_front();
	}

	// Install at the tail
	sw[nid]->TCAMmap[pkt] = sw[nid]->TCAM.push_back(ent);
}
