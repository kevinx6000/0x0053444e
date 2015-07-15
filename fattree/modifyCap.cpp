// Headers
#include "../basic_lib.h"
#include "fattree.h"
#include "../packet/packet.h"
#include "../IP/IP.h"

// Modify the use of capacity along the path
void Fattree::modifyCap(Packet pkt, double chg){

	// Flow ID & data rate
	int flowID = rcdFlowID[pkt];
	double dataRate = pkt.getDataRate()*chg;

	// Wireless policy
	if(allEntry[flowID][0].isWireless()){
		/* Recover original data rate here */
		fprintf(stderr, "(future work) Clear up capacity for wireless\n");
		return;
	}

	// Source IP & ID
	IP srcIP = pkt.getSrcIP();
	int srcID = numberOfCore + numberOfAggregate + numberOfEdge +
		srcIP.byte[1]*pod*pod/4 + srcIP.byte[2]*pod/2 + srcIP.byte[3]-2;

	// Destination IP & ID
	IP dstIP = pkt.getDstIP();
	int dstID = numberOfCore + numberOfAggregate + numberOfEdge +
		dstIP.byte[1]*pod*pod/4 + dstIP.byte[2]*pod/2 + dstIP.byte[3]-2;

	// Get path
	vector<int>path;
	path.push_back(srcID);
	for(int i = 0; i < allEntry[flowID].size(); i++)
		path.push_back(allEntry[flowID][i].getSID());
	path.push_back(dstID);
	
	// For all links on the path
	int port;
	for(int i = 0; i < path.size()-1; i++){
		// Forward
		for(port = 0; port < node[path[i]]->link.size(); port++)
			if(node[path[i]]->link[port].id == path[i+1]) break;
		node[path[i]]->link[port].cap += dataRate;

		// Reverse
		for(port = 0; port < node[path[i+1]]->link.size(); port++)
			if(node[path[i+1]]->link[port].id == path[i]) break;
		node[path[i+1]]->link[port].cap += dataRate;
	}
}
