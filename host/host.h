
// Headers
#include "../node/node.h"
#include "../event/event.h"
#include "../packet/packet.h"

// Host class
#ifndef HOST_H
#define HOST_H
class Host: public Node{
	public:
		Host(int);
		Event forward(double,Packet);
};
#endif
