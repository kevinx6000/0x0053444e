// Headers
#include "fattree.h"
#include "../basic_lib.h"

// Remove expired entries
void Fattree::updateTCAM(int nid, int timeStamp){

	Packet tmpPkt;

	// The most front entries
	while(!sw[nid]->TCAM.empty()){
		if(sw[nid]->TCAM.front().isExpired(timeStamp)){
			tmpPkt = sw[nid]->TCAM.front().getSample();
			sw[nid]->TCAMmap.erase(tmpPkt);
			sw[nid]->TCAM.pop_front();
		}
		else break;
	}
}
