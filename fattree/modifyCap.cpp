// Headers
#include "../basic_lib.h"
#include "fattree.h"
#include "../packet/packet.h"
#include "../IP/IP.h"
#include "../entry/entry.h"

// Modify the use of capacity along the path
void Fattree::modifyCap(Packet pkt, double chg){

	// Flow ID & data rate
	int flowID = rcdFlowID[pkt];
	double dataRate = pkt.getDataRate()*chg;

	// Shift base
	int sft = numberOfCore + numberOfAggregate + numberOfEdge;

	// Source IP & ID
	IP srcIP = pkt.getSrcIP();
	int srcID = sft + srcIP.byte[1]*pod*pod/4 + srcIP.byte[2]*pod/2 + srcIP.byte[3]-2;

	// Destination IP & ID
	IP dstIP = pkt.getDstIP();
	int dstID = sft + dstIP.byte[1]*pod*pod/4 + dstIP.byte[2]*pod/2 + dstIP.byte[3]-2;

	// Source & Destination links
	node[srcID]->link[0].cap += dataRate;
	node[ node[srcID]->link[0].id ]->link[ pod/2 + srcIP.byte[3]-2 ].cap += dataRate;
	node[dstID]->link[0].cap += dataRate;
	node[ node[dstID]->link[0].id ]->link[ pod/2 + dstIP.byte[3]-2 ].cap += dataRate;
	/* Warning: this depends on link creation order and IP naming policy */
	
	// Update along the path
	int nowID, nxtID, port;
	Entry ent;
	for(int i = 0; i < allEntry[flowID].size(); i++){

		// Wireless
		if(allEntry[flowID][i].isWireless()){

			// AP rate of the switch
			nowID = allEntry[flowID][i].getSID();
			sw[nowID]->APrate += dataRate;

			// Last switch, skip
			if(i == allEntry[flowID].size()-1) continue;
			/* However, this is impossible */

			// Interference
			port = allEntry[flowID][i].getOutputPort();
			for(int j = 0; j < sw[nowID]->iList[port].size(); j++){
				sw[ sw[nowID]->iList[port][j] ]->APrate += dataRate;
			}
		}
		
		// Wired
		else{

			// Last switch, skip
			if(i == allEntry[flowID].size()-1) continue;

			// Forward
			ent = allEntry[flowID][i];
			nowID = ent.getSID();
			port = ent.getOutputPort();
			node[nowID]->link[port].cap += dataRate;

			// Reverse
			nxtID = node[nowID]->link[port].id;
			for(port = 0; port < node[nxtID]->link.size(); port++)
				if(node[nxtID]->link[port].id == nowID) break;
			node[nxtID]->link[port].cap += dataRate;
		}
	}
}
