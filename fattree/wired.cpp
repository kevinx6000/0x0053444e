// Headers
#include "fattree.h"
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../entry/entry.h"
#include "../IP/IP.h"

// Wired policy
vector<Entry> Fattree::wired(int nid, Packet pkt){

	// IP
	IP srcIP = pkt.getSrcIP();
	IP dstIP = pkt.getDstIP();

	// Maximum flow entry
	map<int,int>prevNode;
	map<int,int>pathMin;

	// Enumerate all path
	int nowID = nid;
	int dstID;
	int tmpMax;
	int pathLen = 1;
	pathMin[nowID] = 0;
	queue<int>BFS;
	BFS.push(nowID);
	pathInit(pkt, prevNode);

	// Edge -> Aggregate
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){
		nowID = BFS.front();
		BFS.pop();
		for(int i = 0; i < pod/2; i++){
			dstID = node[nowID]->link[i].id;
			pathMin[dstID] = copyTCAM[nowID].size();
			BFS.push(dstID);
			prevNode[dstID] = nowID;
		}
		pathLen ++;
	}

	// Aggregate -> Core
	if(srcIP.byte[1] != dstIP.byte[1]){
		for(int i = 0; i < pod/2; i++){
			nowID = BFS.front();
			BFS.pop();
			for(int j = 0; j < pod/2; j++){
				dstID = node[nowID]->link[j].id;
				pathMin[dstID] = myMax(copyTCAM[nowID].size(), pathMin[nowID]);
				BFS.push(dstID);
				prevNode[dstID] = nowID;
			}
		}
		pathLen ++;
	}

	// Core -> Aggregate
	int siz;
	if(srcIP.byte[1] != dstIP.byte[1]){
		for(int i = 0; i < pod*pod/4; i++){
			nowID = BFS.front();
			BFS.pop();
			dstID = node[nowID]->link[ dstIP.byte[1] ].id;
			if(!(i%(pod/2))){
				pathMin[dstID] = myMax(copyTCAM[nowID].size(), pathMin[nowID]);
				prevNode[dstID] = nowID;
				BFS.push(dstID);
				siz = copyTCAM[nowID].size();
			}
			else{
				tmpMax = myMax(copyTCAM[nowID].size(), pathMin[nowID]);
				if(tmpMax < pathMin[dstID]){
					pathMin[dstID] = tmpMax;
					siz = copyTCAM[nowID].size();
					prevNode[dstID] = nowID;
				}
				else if(tmpMax == pathMin[dstID] && copyTCAM[nowID].size() < siz){
					siz = copyTCAM[nowID].size();
					prevNode[dstID] = nowID;
				}
			}
		}
		pathLen ++;
	}

	// Aggregate -> Edge
	if(srcIP.byte[1] != dstIP.byte[1] || srcIP.byte[2] != dstIP.byte[2]){
		for(int i = 0; i < pod/2; i++){
			nowID = BFS.front();
			BFS.pop();
			dstID = node[nowID]->link[ pod/2 + dstIP.byte[2] ].id;
			if(!i){
				pathMin[dstID] = myMax(copyTCAM[nowID].size(), pathMin[nowID]);
				prevNode[dstID] = nowID;
				BFS.push(dstID);
				siz = copyTCAM[nowID].size();
			}
			else{
				tmpMax = myMax(copyTCAM[nowID].size(), pathMin[nowID]);
				if(tmpMax < pathMin[dstID]){
					pathMin[dstID] = tmpMax;
					siz = copyTCAM[nowID].size();
					prevNode[dstID] = nowID;
				}
				else if(tmpMax == pathMin[dstID] && copyTCAM[nowID].size() < siz){
					siz = copyTCAM[nowID].size();
					prevNode[dstID] = nowID;
				}
			}
		}
		pathLen ++;
	}

	// Create entries along these switches
	nowID = BFS.front();
	BFS.pop();
	vector<int>revSeq;
	vector<Entry>vent;
	for(int i = 0; i < pathLen; i++){
		revSeq.push_back(nowID);
		nowID = prevNode[nowID];
	}
	int port;
	Entry ent;
	ent.setDstMask(dstIP.byte[0], dstIP.byte[1], dstIP.byte[2], dstIP.byte[3]);
	for(int i = pathLen-1; i >0 ; i--){
		for(port = 0; port < node[revSeq[i]]->link.size(); port++)
			if(node[revSeq[i]]->link[port].id == revSeq[i-1]) break;
		ent.setSID(revSeq[i]);
		ent.setOutputPort(port);
		vent.push_back(ent);
	}
	ent.setSID(revSeq[0]);
	ent.setOutputPort(pod/2 + dstIP.byte[3] - 2);
	vent.push_back(ent);

	return vent;
}
