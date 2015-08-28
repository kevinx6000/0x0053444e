
// Headers
#include "../basic_lib.h"
#include "../event/event.h"
#include "../event/eventType.h"
#include "../packet/packet.h"
#include "../IP/IP.h"
#include "../switch/switch.h"

// Switch class
Switch::Switch(int id):Node(id){
}
Event Switch::forward(double timeStamp, Packet pkt){

	// Variables
	double TCAMDelay = TCAM_SEARCH_DELAY;
	double forwardDelay;
	int outputPort;
	bool arrive;
	Packet tmpPkt;
	Event evt;
	IP dstIP;
	IP selfIP;

	// Arrival of destination
	arrive = true;
	selfIP = this->getIP();
	dstIP = pkt.getDstIP();
	for(int i = 0; i < 4; i++){
		if(dstIP.byte[i] != selfIP.byte[i]){
			arrive = false;
			break;
		}
	}
	if(arrive){
		evt.setTimeStamp(timeStamp);
		evt.setEventType(EVENT_DONE);
		evt.setPacket(pkt);
		return evt;
	}

	// Remove expired entries (Active & inactive)
	while(!TCAMactive.empty()){
		if(TCAMactive.front().isExpired(timeStamp)){
			tmpPkt = TCAMactive.front().getSample();
			TCAMmapA.erase(tmpPkt);
			TCAMactive.pop_front();
		}
		else break;
	}
	while(!TCAMinactive.empty()){
		if(TCAMinactive.front().isExpired(timeStamp)){
			tmpPkt = TCAMinactive.front().getSample();
			TCAMmapI.erase(tmpPkt);
			TCAMinactive.pop_front();
		}
		else break;
	}

	// Search in TCAM
	int pri = -1;
	Entry result;
	if(TCAMmapA.count(pkt) > 0){
		result = TCAMmapA[pkt]->ent;
		pri = 0;

		// Update entry expire time & Install at the tail (LRU)
		result.setExpire(timeStamp + ENTRY_EXPIRE_TIME);
		TCAMactive.remove(TCAMmapA[pkt]);
		TCAMmapA[pkt] = TCAMactive.push_back(result);
	}
	else if(TCAMmapI.count(pkt) > 0){
		result = TCAMmapI[pkt]->ent;
		pri = 0;

		// Remove from inactive
		TCAMinactive.remove(TCAMmapI[pkt]);
		TCAMmapI.erase(pkt);
		
		// Update timestamp &  Install at the tail (LRU)
		result.setExpire(timeStamp + ENTRY_EXPIRE_TIME);
		TCAMmapA[pkt] = TCAMactive.push_back(result);
	}

	// Entry not found
	if(pri == -1){

		// Check if this flow already requesting flow setup
		if(isSetup[pkt]){

			// Push into queue
			que.push_back(pkt);

			// No OP
			evt.setTimeStamp(timeStamp);
			evt.setEventType(EVENT_NOP);
			return evt;
		}

		// Flow setup request
		else{
			// Record & put into queue
			isSetup[pkt] = true;
			que.push_back(pkt);

			// TCAM searching delay
			evt.setTimeStamp(timeStamp + TCAMDelay);
			evt.setEventType(EVENT_FLOWSETUP);
			evt.setPacket(pkt);
			evt.setID(this->getID());
			return evt;
		}
	}

	// Clear
	isSetup[pkt] = false;

	// Foward after link available
	outputPort = result.getOutputPort();
	timeStamp = timeStamp + TCAMDelay;

	// Forward event
	forwardDelay = pkt.getFlowSize() / pkt.getDataRate();
	evt.setTimeStamp(timeStamp + forwardDelay);
	evt.setEventType(EVENT_FORWARD);
	evt.setPacket(pkt);

	// Wired only
	evt.setID(link[result.getOutputPort()].id);

	// Return event
	return evt;
}
