
// Headers
#include "../basic_lib.h"
#include "../packet/packet.h"
#include "../node/node.h"
#include "../switch/core.h"
#include "../switch/aggregate.h"
#include "../switch/edge.h"
#include "../host/host.h"
#include "../fattree/fattree.h"
#include "../event/event.h"
#include "../event/eventType.h"
#include "../entry/entry.h"

// Fat Tree class
Fattree::Fattree(int k){

	// TCAM entries
	maxEntry = 100;

	// Controller available time
	compAvail = 0.0;

	// 1 feet = 0.3048 meter
	// 1 inch = 0.0254 meter
	double feet = 0.3048;
	double inch = 0.0254;
	
	// Width/Length of switch
	double widSw = 24*inch;
	double lenSw = 48*inch;
	
	// Prevent illegal k
	if(k<2) k=2;
	if(k%2) k--;
	pod = k;

	// Number of nodes
	numberOfCore = (k/2)*(k/2);
	numberOfAggregate = k*k/2;
	numberOfEdge = numberOfAggregate;
	numberOfHost = k*k*k/4;
	totalNode = numberOfCore + numberOfAggregate + numberOfEdge + numberOfHost;
	node = new Node*[totalNode];
	sw = new Switch*[totalNode - numberOfHost];

	// All path flow entry reset
	flowIDCount = 1;
	vector<Entry>vent;
	allEntry.push_back(vent);
	rcdFlowID.clear();

	// Core
	IP ip;
	int now = 0;
	for(int i = 0; i < numberOfCore; i++){
		sw[now] = new Core(now);
		ip.setIP(10, pod, i/(pod/2)+1, i%(pod/2)+1);
		sw[now]->setIP(ip);
		node[now] = sw[now];
		copyTCAM.push_back(vent);
		now++;
	}

	// Aggregate
	for(int i = 0; i < numberOfAggregate; i++){
		sw[now] = new Aggregate(now);
		ip.setIP(10, i/(pod/2), i%(pod/2)+(pod/2), 1);
		sw[now]->setIP(ip);
		node[now] = sw[now];
		copyTCAM.push_back(vent);
		now++;
	}

	// Edge
	for(int i = 0; i < numberOfEdge; i++){
		sw[now] = new Edge(now);
		ip.setIP(10, i/(pod/2), i%(pod/2), 1);
		sw[now]->setIP(ip);
		node[now] = sw[now];
		copyTCAM.push_back(vent);

		// Position X, Y
		sw[now]->posXY[0] = (i%10)*widSw + 0.5*widSw + ((i/10)%4)*(10*feet+10*widSw);
		sw[now]->posXY[1] = 0.5*lenSw + (i/40)*(lenSw+8*feet);
		now++;
	}

	// Host
	for(int i = 0; i < numberOfHost; i++){
		node[now] = new Host(now);
		ip.setIP(10, i/(pod*pod/4), (i%(pod*pod/4))/(pod/2), (i%(pod*pod/4))%(pod/2)+2);
		node[now]->setIP(ip);
		now++;
	}

	// Link initialization
	int src, dst;
	EDGE etmp;
	etmp.cap = LINK_CAPACITY;

	// Core - aggregrate links
	for(int i = 0; i < numberOfAggregate; i++)
		for(int j = 0; j < pod/2; j++){
			src = numberOfCore + i;
			dst = (i%(pod/2))*pod/2 + j;
			etmp.id = dst;
			node[src]->link.push_back(etmp);
			etmp.id = src;
			node[dst]->link.push_back(etmp);
			node[src]->avail.push_back(0.0);
			node[dst]->avail.push_back(0.0);
		}
	
	// Aggregate - Edge links
	for(int i = 0; i < numberOfAggregate; i++)
		for(int j = 0; j < pod/2; j++){
			src = numberOfCore + i;
			dst = numberOfCore + numberOfAggregate + (i/(pod/2))*pod/2 + j;
			etmp.id = dst;
			node[src]->link.push_back(etmp);
			etmp.id = src;
			node[dst]->link.push_back(etmp);
			node[src]->avail.push_back(0.0);
			node[dst]->avail.push_back(0.0);
		}

	// Edge - Host links
	for(int i = 0; i < numberOfEdge; i++)
		for(int j = 0; j < pod/2; j++){
			src = numberOfCore + numberOfAggregate + i;
			dst = numberOfCore + numberOfAggregate + numberOfEdge + i*(pod/2) + j;
			etmp.id = dst;
			node[src]->link.push_back(etmp);
			etmp.id = src;
			node[dst]->link.push_back(etmp);
			node[src]->avail.push_back(0.0);
			node[dst]->avail.push_back(0.0);
		}
	
	// Random seeds
	srand((unsigned)time(NULL));

	// Controller interval timeout event
	Event evt;
	evt.setEventType(EVENT_INTERVAL);
	evt.setTimeStamp(10.0);
	eventQueue.push(evt);

// DEBUG
//for(int i = numberOfCore + numberOfAggregate; i < totalNode - numberOfHost; i++){
//	printf("Switch %2d: %.2lf/%.2lf\n", sw[i]->getID(), sw[i]->posXY[0], sw[i]->posXY[1]);
//}
}
