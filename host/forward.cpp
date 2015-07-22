
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
	double forwardDelay;
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
		evt.setEventType(EVENT_DONE);
		evt.setPacket(pkt);
//printf("[%6.1lf] Flow %d arrives at destination.\n", timeStamp, pkt.getSequence());
		return evt;
	}
	
	// Forward to switch
	forwardDelay = pkt.getFlowSize() / pkt.getDataRate();
	evt.setTimeStamp(timeStamp + forwardDelay);
	evt.setEventType(EVENT_FORWARD);
	evt.setID(link[0].id);
	evt.setPacket(pkt);

	return evt;
}
