// Headers
#include "fattree.h"
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../entry/entry.h"
#include "../IP/IP.h"

// Wired policy
bool Fattree::wired(int nid, Packet pkt, vector<Entry>& vent){

	// IP
	IP srcIP = pkt.getSrcIP();
	IP dstIP = pkt.getDstIP();

	// Maximum flow entry
	map<int,int>prevNode;
	map<int,int>pathMin;

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
	pathMin[srcID] = 0;
	endID = pathInit(pkt, prevNode);
	prevNode[srcID] = srcID;

	// Edge -> Aggregate
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){
		nowID = BFS.front();
		BFS.pop();
		for(int i = 0; i < pod/2; i++){
			if(node[nowID]->link[i].cap >= dataRate){
				dstID = node[nowID]->link[i].id;
				pathMin[dstID] = sw[nowID]->TCAM.size();
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
			for(int j = 0; j < pod/2; j++){
				if(node[nowID]->link[j].cap >= dataRate){
					dstID = node[nowID]->link[j].id;
					pathMin[dstID] = myMax(sw[nowID]->TCAM.size(), pathMin[nowID]);
					BFS.push(dstID);
					prevNode[dstID] = nowID;
				}
			}
		}
	}

	// Core -> Aggregate
	int siz;
	if(srcIP.byte[1] != dstIP.byte[1]){
		queSiz = BFS.size();
		for(int i = 0; i < queSiz; i++){
			nowID = BFS.front();
			BFS.pop();
			if(node[nowID]->link[ dstIP.byte[1] ].cap >= dataRate){
				dstID = node[nowID]->link[ dstIP.byte[1] ].id;
				if(prevNode[dstID]==-1){
					pathMin[dstID] = myMax(sw[nowID]->TCAM.size(), pathMin[nowID]);
					prevNode[dstID] = nowID;
					BFS.push(dstID);
					siz = sw[nowID]->TCAM.size();
				}
				else{
					tmpMax = myMax(sw[nowID]->TCAM.size(), pathMin[nowID]);
					if(tmpMax < pathMin[dstID]){
						pathMin[dstID] = tmpMax;
						siz = sw[nowID]->TCAM.size();
						prevNode[dstID] = nowID;
					}
					else if(tmpMax == pathMin[dstID] && sw[nowID]->TCAM.size() < siz){
						siz = sw[nowID]->TCAM.size();
						prevNode[dstID] = nowID;
					}
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
			if(node[nowID]->link[ pod/2 + dstIP.byte[2] ].cap >= dataRate){
				dstID = node[nowID]->link[ pod/2 + dstIP.byte[2] ].id;
				if(prevNode[dstID]==-1){
					pathMin[dstID] = myMax(sw[nowID]->TCAM.size(), pathMin[nowID]);
					prevNode[dstID] = nowID;
					BFS.push(dstID);
					siz = sw[nowID]->TCAM.size();
				}
				else{
					tmpMax = myMax(sw[nowID]->TCAM.size(), pathMin[nowID]);
					if(tmpMax < pathMin[dstID]){
						pathMin[dstID] = tmpMax;
						siz = sw[nowID]->TCAM.size();
						prevNode[dstID] = nowID;
					}
					else if(tmpMax == pathMin[dstID] && sw[nowID]->TCAM.size() < siz){
						siz = sw[nowID]->TCAM.size();
						prevNode[dstID] = nowID;
					}
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
		ent.setDstMask(dstIP.byte[0], dstIP.byte[1], dstIP.byte[2], dstIP.byte[3]);
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
