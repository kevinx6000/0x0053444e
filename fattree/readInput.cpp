
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

// Read packet/flow
void Fattree::readInput(void){

	// Variables
	int byte[4], srcPort, dstPort, protocol, timeStamp, seq;
	char charSrcIP[20], charDstIP[20];
	IP dstIP, srcIP;
	Event evt;
	Packet pkt;

	// Packets (5-tuples and arrival time)
	seq = 1;
	while(scanf("%s %s %d %d %d %d", charSrcIP, charDstIP, &srcPort, &dstPort, &protocol, &timeStamp) == 6){

		// Setup Packet Info
		srcIP.setIP(charSrcIP);
		dstIP.setIP(charDstIP);
		pkt.setSrcIP(srcIP);
		pkt.setDstIP(dstIP);
		pkt.setSrcPort(srcPort);
		pkt.setDstPort(dstPort);
		pkt.setProtocol(protocol);
		pkt.setSequence(seq++);

		// Put into event queue
		evt.setTimeStamp(timeStamp);
		evt.setEventType(EVENT_FORWARD);
		evt.setPacket(pkt);
		evt.setID(numberOfCore + numberOfAggregate + numberOfEdge + 
					srcIP.byte[1]*pod*pod/4 + srcIP.byte[2]*pod/2 + srcIP.byte[3]-2);
		eventQueue.push(evt);
	}
}
