
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
		printf("[%6.1lf] Packet %s arrives at destination.\n", timeStamp, pkt.getDstIP().fullIP.c_str());
		return evt;
	}

	// Search in TCAM
	int pri = -1, pp;
	Entry result;
	for(int i = 0; i < TCAM.size(); i++){
		if(TCAM[i].isMatch(pkt)){
			pp = TCAM[i].getPriority();
			if(pp > pri){
				pri = pp;
				result = TCAM[i];
			}
		}
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
	forwardDelay = pkt.getNumPkt()*PACKET_SIZE / pkt.getDataRate();
	evt.setTimeStamp(timeStamp + forwardDelay);
	evt.setEventType(EVENT_FORWARD);
	evt.setPacket(pkt);
	evt.setID(link[result.getOutputPort()].id);

	// Return event
	return evt;
}
