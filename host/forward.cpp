
// Headers
#include "../basic_lib.h"
#include "../event/event.h"
#include "../event/eventType.h"
#include "../host/host.h"
#include "../IP/IP.h"

// Host class
Event Host::forward(double timeStamp, Packet pkt){

	// Variables
	bool arrive;
	double transDelay = 1.0;
	IP selfIP, dstIP;
	Event evt;

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
		evt.setEventType(EVENT_NOP);
		printf("[%6.1lf] Packet %d arrives at destination.\n", timeStamp, pkt.getSequence());
		return evt;
	}
	
	// Link available time
	timeStamp = max(timeStamp, avail[0]);
	
	// Forward to switch
	evt.setTimeStamp(timeStamp + transDelay);
	evt.setEventType(EVENT_FORWARD);
	evt.setID(link[0]);
	evt.setPacket(pkt);

	// Update available time
	avail[0] = timeStamp + transDelay;
	return evt;
}
