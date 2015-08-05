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

	// Maximum flow entry
	map<int,int>prevNode;

	// Enumerate all path
	int nowID;
	int srcID = nid;
	int dstID;
	int tmpMax;
	int queSiz;
	int endID;
	double dataRate = pkt.getDataRate();
	queue<int>BFS;
	BFS.push(srcID);
	endID = pathInit(pkt, prevNode);
	prevNode[srcID] = srcID;

	// Edge -> Aggregate
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){
		nowID = BFS.front();
		BFS.pop();

		// Remove expired entries
		updateTCAM(nowID, timeStamp);
		for(int i = 0; i < pod/2; i++){
			if(node[nowID]->link[i].cap >= dataRate){
				dstID = node[nowID]->link[i].id;
				BFS.push(dstID);
				prevNode[dstID] = nowID;
			}
		}
	}

	// Aggregate -> Core
	if(srcIP.byte[1] != dstIP.byte[1]){
		queSiz = BFS.size();
		for(int i = 0; i < queSiz; i++){
			nowID = BFS.front();
			BFS.pop();

			// Remove expired entries
			updateTCAM(nowID, timeStamp);
			for(int j = 0; j < pod/2; j++){
				if(node[nowID]->link[j].cap >= dataRate){
					dstID = node[nowID]->link[j].id;
					BFS.push(dstID);
					prevNode[dstID] = nowID;
				}
			}
		}
	}

	// Core -> Aggregate
	if(srcIP.byte[1] != dstIP.byte[1]){
		queSiz = BFS.size();
		for(int i = 0; i < queSiz; i++){
			nowID = BFS.front();
			BFS.pop();
			
			// Remove expired entries
			updateTCAM(nowID, timeStamp);
			if(node[nowID]->link[ dstIP.byte[1] ].cap >= dataRate){
				dstID = node[nowID]->link[ dstIP.byte[1] ].id;

				// First one
				if(prevNode[dstID]==-1){
					prevNode[dstID] = nowID;
					BFS.push(dstID);
				}
				else{
					// Random pick
					if(rand()%2)
						prevNode[dstID] = nowID;
				}
			}
		}
	}

	// Aggregate -> Edge
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){
		queSiz = BFS.size();
		for(int i = 0; i < queSiz; i++){
			nowID = BFS.front();
			BFS.pop();

			// Remove expired entries
			updateTCAM(nowID, timeStamp);
			if(node[nowID]->link[ pod/2 + dstIP.byte[2] ].cap >= dataRate){
				dstID = node[nowID]->link[ pod/2 + dstIP.byte[2] ].id;

				// First one
				if(prevNode[dstID]==-1){
					prevNode[dstID] = nowID;
					BFS.push(dstID);
				}
				else{
					// Randomly pick
					if(rand()%2)
						prevNode[dstID] = nowID;
				}	
			}
		}
	}

	// Create entries along these switches
	int port;
	Entry ent;
	vector<int>revSeq;
	if(prevNode[endID]!=-1){
		vent.clear();
		nowID = endID;
		revSeq.push_back(endID);
		while(nowID != srcID){
			nowID = prevNode[nowID];
			revSeq.push_back(nowID);
		}
		ent.setSrcMask(srcIP.byte[0], srcIP.byte[1], srcIP.byte[2], srcIP.byte[3]);
		ent.setDstMask(dstIP.byte[0], dstIP.byte[1], dstIP.byte[2], dstIP.byte[3]);
		ent.setSrcPort(pkt.getSrcPort());
		ent.setDstPort(pkt.getDstPort());
		ent.setProtocol(pkt.getProtocol());
		ent.setExpire(timeStamp + ENTRY_EXPIRE_TIME);
		for(int i = revSeq.size()-1; i > 0; i--){
			for(port = 0; port < node[revSeq[i]]->link.size(); port++)
				if(node[revSeq[i]]->link[port].id == revSeq[i-1]) break;
			ent.setSID(revSeq[i]);
			ent.setOutputPort(port);
			vent.push_back(ent);
		}
		ent.setSID(revSeq[0]);
		ent.setOutputPort(pod/2 + dstIP.byte[3] - 2);
		vent.push_back(ent);
		return true;
	}

	// No such path
	else return false;
}
