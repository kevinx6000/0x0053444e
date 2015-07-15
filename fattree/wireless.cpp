// Headers
#include "fattree.h"
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../entry/entry.h"
#include "../IP/IP.h"

// Wireless policy
bool Fattree::wireless(int nid, Packet pkt, vector<Entry>& vent){

	// IP
	IP srcIP = pkt.getSrcIP();
	IP dstIP = pkt.getDstIP();

	// IDs
	int hostID = numberOfCore + numberOfAggregate + numberOfEdge +
		dstIP.byte[1]*pod*pod/4 + dstIP.byte[2]*pod/2 + dstIP.byte[3]-2;
	int srcID = nid;
	int dstID = node[hostID]->link[0].id;
	int nowID, nxtID;
	int sft = numberOfCore + numberOfAggregate;
	int ss = srcID - sft;
	int tt = dstID - sft;

	// No such path
	if(!wlPath[ss][tt].size()) return false;

	// Check if capacity fit along the path
	fprintf(stderr, "(future work) Check if capacity fit along the path\n");
	/* This is future work */

	// Create entries along these switches
	int port;
	Entry ent;
	ent.isWireless(true);
	ent.setDstMask(dstIP.byte[0], dstIP.byte[1], dstIP.byte[2], dstIP.byte[3]);
	vent.clear();
	for(int i = 0; i < wlPath[ss][tt].size()-1; i++){
		nowID = wlPath[ss][tt][i];
		nxtID = wlPath[ss][tt][i+1];
		ent.setSID(nowID);
		for(port = 0; port < sw[nowID]->wlink.size(); port++)
			if(sw[nowID]->wlink[port].id == nxtID) break;
		ent.setOutputPort(port);
		vent.push_back(ent);
	}
	ent.isWireless(false);
	ent.setSID(wlPath[ss][tt][ wlPath[ss][tt].size()-1 ]);
	ent.setOutputPort(pod/2 + dstIP.byte[3] - 2);
	vent.push_back(ent);
	return true;
}
