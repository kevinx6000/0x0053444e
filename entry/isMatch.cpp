
// Headers
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../entry/entry.h"
#include "../IP/IP.h"

bool Entry::isMatch(Packet pkt){

	// IPs
	IP srcIP, dstIP;
	srcIP = pkt.getSrcIP();
	dstIP = pkt.getDstIP();
	
	// Source IP subnet
	bool ok = true;
	for(int i = 0; i < 4; i++)
		if(srcMask[i] != -1 && srcMask[i] != srcIP.byte[i]){
			ok = false;
			break;
		}
	
	// Destination IP subnet
	for(int i = 0; i < 4; i++)
		if(dstMask[i] != -1 && dstMask[i] != dstIP.byte[i]){
			ok=false;
			break;
		}

	// Matching result
	return ok;
}
