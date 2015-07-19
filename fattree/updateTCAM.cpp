// Headers
#include "fattree.h"
#include "../basic_lib.h"

// Remove expired entries
void Fattree::updateTCAM(int nid, int timeStamp){
	for(int i = 0; i < sw[nid]->TCAM.size(); i++){
		if(sw[nid]->TCAM[i].isExpired(timeStamp)){
			sw[nid]->TCAM.erase(sw[nid]->TCAM.begin()+i);
			i--;
		}
	}
}
