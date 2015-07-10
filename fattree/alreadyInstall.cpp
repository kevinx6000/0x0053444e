// Header
#include "fattree.h"
#include "../event/event.h"
#include "../packet/packet.h"

// Flow Setup Needed or not
bool Fattree::alreadyInstall(Event evt){
	int nid = evt.getID();
	Packet pkt = evt.getPacket();
	for(int i = 0; i < copyTCAM[nid].size(); i++)
		if(copyTCAM[nid][i].isMatch(pkt))
			return false;
	return true;
}
