
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
	int sid, siz, arrive;
	double ts;
	Event evt, next;
	PrevHop ph;
	map<int,PrevHop>::iterator itr;
	pair<Event,Event>pr;

	arrive = 0;
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

				// Has previous hop record
				itr = this->prevHop.find(evt.getPacket().getSequence());
				if(itr != this->prevHop.end()){

					// Release capacity
					modCap(evt.getID(), evt.getPacket().getSequence(), evt.getPacket().getDataRate());
					ph = itr->second;

					// Check blocked flows
					resumeFlow(ph.id, evt.getTimeStamp());

					// Remove that record
					this->prevHop.erase(itr);
				}

				// Try to forward
				next = node[evt.getID()]->forward(evt.getTimeStamp(), evt.getPacket());

				// Forward event
				if(next.getEventType() == EVENT_FORWARD){

					// Blocked
					if(blockFlow(evt, next)){

						// Store into queue
						printf("[%6.1lf] Block: %d at %d.\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
						pr.first = evt;
						pr.second = next;
						node[evt.getID()]->blockEvt.push_back(pr);
						break;
					}
	
					else{
						// Record previous hop and consume capacity
						recrdPrev(evt, next);
						modCap(evt.getID(), evt.getPacket().getSequence(), evt.getPacket().getDataRate()*(-1.0));
					}
				}

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
				arrive ++;
				printf("[%6.1lf] %d flows arrives\n", evt.getTimeStamp(), arrive);
				break;

			// Unknown
			case EVENT_UNKNOWN:
				printf("Error: unknown operation found.\n");
				break;
		}
	}
}
