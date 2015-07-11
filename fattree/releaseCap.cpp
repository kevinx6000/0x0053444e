// Headers
#include "../basic_lib.h"
#include "fattree.h"
#include "../packet/packet.h"

// Release Used Capacity along the path
void Fattree::releaseCap(Packet pkt){

	// Flow ID
	int flowID = rcdFlowID[pkt];

	// Get path
	vector<int>path;
	for(int i = 0; i < allEntry[flowID].size(); i++)
		path.push_back(allEntry[flowID][i].getSID());

	// DEBUG
	printf("PATH DEBUG:\n");
	for(int i = 0; i < path.size(); i++)
		printf(" %d", path[i]);
	printf("\n");
}
