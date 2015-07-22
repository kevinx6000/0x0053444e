// Headers
#include "fattree.h"
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../entry/entry.h"
#include "../IP/IP.h"

// Wired policy
bool Fattree::wired(int nid, Packet pkt, vector<Entry>& vent, int timeStamp){

	// IP
	IP srcIP = pkt.getSrcIP();
	IP dstIP = pkt.getDstIP();

	// Variables
	int nowID;
	int srcID = nid;
	int dstID;
	int tmpMax;
	int queSiz;
	int endID;
	double dataRate = pkt.getDataRate();
	vector<int>ranPath;

	// Starting edge switch
	ranPath.push_back(srcID);

	// Edge -> Aggregate
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){

		// Randomly pick aggregate switch
		nowID = ranPath[ ranPath.size()-1 ];
		ranPath.push_back(node[nowID]->link[rand()%(pod/2)].id);
	}

	// Aggregate -> Core
	if(srcIP.byte[1] != dstIP.byte[1]){

		// Randomly pick core switch
		nowID = ranPath[ ranPath.size()-1 ];
		ranPath.push_back(node[nowID]->link[rand()%(pod/2)].id);
	}

	// Core -> Aggregate
	int siz;
	if(srcIP.byte[1] != dstIP.byte[1]){

		// Unique aggreggate switch
		nowID = ranPath[ ranPath.size()-1 ];
		ranPath.push_back(node[nowID]->link[ dstIP.byte[1] ].id);
	}

	// Aggregate -> Edge
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){

		// Unique edge switch
		nowID = ranPath[ ranPath.size()-1 ];
		ranPath.push_back(node[nowID]->link[ pod/2 + dstIP.byte[2] ].id);
	}

	// Create entries along these switches
	int port;
	Entry ent;
	vent.clear();
	ent.setSrcMask(srcIP.byte[0], srcIP.byte[1], srcIP.byte[2], srcIP.byte[3]);
	ent.setDstMask(dstIP.byte[0], dstIP.byte[1], dstIP.byte[2], dstIP.byte[3]);
	ent.setSrcPort(pkt.getSrcPort());
	ent.setDstPort(pkt.getDstPort());
	ent.setProtocol(pkt.getProtocol());
	ent.setExpire(timeStamp + ENTRY_EXPIRE_TIME);
	for(int i = 0; i < ranPath.size()-1; i++){
		for(port = 0; port < node[ ranPath[i] ]->link.size(); port++)
			if(node[ ranPath[i] ]->link[port].id == ranPath[i+1]) break;
		ent.setSID(ranPath[i]);
		ent.setOutputPort(port);
		vent.push_back(ent);
	}
	ent.setSID(ranPath[ ranPath.size()-1 ]);
	ent.setOutputPort(pod/2 + dstIP.byte[3] - 2);
	vent.push_back(ent);
	return true;
}
