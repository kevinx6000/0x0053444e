
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

// Start simulation
void Fattree::start(void){

	// Until all event finished
	int sid, siz;
	double ts;
	Event evt, next;
	while(!eventQueue.empty()){
	
		// Get current event
		evt = eventQueue.top();
		eventQueue.pop();

		// Process event
		switch(evt.getEventType()){

			// No operation
			case EVENT_NOP:
				printf("[%6.1lf] No operation.\n", evt.getTimeStamp());
				break;

			// Forwarding
			case EVENT_FORWARD:
				printf("[%6.1lf] Forward: %d at %d.\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());

				// Release capacity of previous hop, and remove that record
				modCap(evt.getID(), evt.getPacket().getSequence(), evt.getPacket().getDataRate());
				this->prevHop.erase(evt.getPacket().getSequence());

				/* Check if some flow can forward now */
				
				// Try to forward
				next = node[evt.getID()]->forward(evt.getTimeStamp(), evt.getPacket());
				/* Here we have to check the capacity, and create QUEUE event if forward failed */

				/* If this is an forwarding event, record previous switch/port and consume capacity */
				if(next.getEventType() == EVENT_FORWARD){
					recrdPrev(evt, next);
					modCap(evt.getID(), evt.getPacket().getSequence(), evt.getPacket().getDataRate()*(-1.0));
				}
				
				/* If forwarding failed, queue and record switch/port/wired.wireless */
				/* Create another event */

				// Push into event queue
				eventQueue.push(next);
				break;

			// Cumulate until interval timeout
			case EVENT_FLOWSETUP:
				printf("[%6.1lf] Flow setup request: %d at %d.\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
				cumulate(evt);
				break;

			// Interval timeout: handle batch of flow setup requests
			case EVENT_INTERVAL:
				printf("[%6.1lf] Handle batch of request by controller.\n", evt.getTimeStamp());
				controller(evt);
				break;

			// Install & forward
			case EVENT_INSTALL:
				printf("[%6.1lf] Install: %d at %d\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
				install(evt);

				// Check the queue of corresponding switch
				sid = evt.getID();
				ts = evt.getTimeStamp();
				siz = sw[sid]->TCAM.size();
				for(int i = 0; i < sw[sid]->que.size(); i++){

					// Only check last entry of TCAM
					if(sw[sid]->TCAM[siz-1].isMatch(sw[sid]->que[i])){

						// Forward that packet
						next.setTimeStamp(ts);
						next.setEventType(EVENT_FORWARD);
						next.setID(sid);
						next.setPacket(sw[sid]->que[i]);
						eventQueue.push(next);

						// Remove from queue
						sw[sid]->que.erase(sw[sid]->que.begin()+i);
						i--;
					}
				}
				break;

			// Flow transmission done
			case EVENT_DONE:
				//modifyCap(evt.getPacket(), +1);
				break;

			// Unknown
			case EVENT_UNKNOWN:
				printf("Error: unknown operation found.\n");
				break;
		}
	}
}
