
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
	int sid, arrive, totFlow;
	double ts;
	Event evt, next;
	PrevHop ph;
	map<int,PrevHop>::iterator itr;
	pair<Event,Event>pr;

	// Statistic information
	int prevPerCent = -1, perCent;
	int numberOfWiredFlow = 0;
	int numberOfWirelessFlow = 0;
	int nowFlowID;
	arrive = 0;
	totFlow = ((int)eventQueue.size()) - 1;

	// Event queue
	while(!eventQueue.empty()){

		// Get current event
		evt = eventQueue.top();
		eventQueue.pop();

		// Process event
		switch(evt.getEventType()){

			// No operation
			case EVENT_NOP:
//printf("[%6.1lf] No operation.\n", evt.getTimeStamp());
				break;

			// Forwarding
			case EVENT_FORWARD:
//printf("[%6.1lf] Forward: %d at %d.\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());

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

					// Record number of flow for statistic info
					/* At starting edge only */
					if(evt.getID() < numberOfCore + numberOfAggregate + numberOfEdge){
						nowFlowID = rcdFlowID[evt.getPacket()];
						if(evt.getID() == allEntry[nowFlowID][0].getSID()){

							// Up to aggr or core only
							if(evt.getPacket().getSrcIP().byte[1] != evt.getPacket().getDstIP().byte[1]
									|| evt.getPacket().getSrcIP().byte[2] != evt.getPacket().getDstIP().byte[2]){
								if(allEntry[nowFlowID][0].isWireless())	numberOfWirelessFlow ++;
								else numberOfWiredFlow ++;
							}
						}
					}

					// Hosts
					else{
						// Update alive flows
						begTransmission(evt.getTimeStamp(), evt.getPacket());
					}

					// Blocked
					if(blockFlow(evt, next)){

						// Store into queue
//printf("[%6.1lf] Block: %d at %d.\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
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
//printf("[%6.1lf] Flow setup request: %d at %d.\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
				cumulate(evt);
				metric_flowSetupRequest ++;
				break;

			// Interval timeout: handle batch of flow setup requests
			case EVENT_INTERVAL:
				controller(evt);
				break;

			// Install & forward
			case EVENT_INSTALL:
//printf("[%6.1lf] Install: %d at %d\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
				install(evt);
				metric_ruleInstallCount ++;

				// Check the queue of corresponding switch
				sid = evt.getID();
				ts = evt.getTimeStamp();
				for(int i = 0; i < sw[sid]->que.size(); i++){

					// Only check last entry of TCAM
					if(sw[sid]->TCAMactive.back().isMatch(sw[sid]->que[i])){

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
//printf("[%6.1lf] %d flows arrives\n", evt.getTimeStamp(), arrive);
				
				// Update alive flows
				endTransmission(evt.getTimeStamp(), evt.getPacket());

				// Percentage
				arrive ++;
				perCent = (arrive*100)/totFlow;
				if(perCent != prevPerCent){
					printf("%3d%% (%d/%d) done.\n", perCent, arrive, totFlow);
					prevPerCent = perCent;
				}
	
				// Flow arrival time
				metric_avgFlowCompleteTime += (evt.getTimeStamp() - metric_flowArrivalTime[evt.getPacket().getSequence()]);
				metric_flowArrivalTime.erase(evt.getPacket().getSequence());

				// Output metric
				if(perCent == 100){
					printf("# of flow setup request: %d\n", metric_flowSetupRequest);
					printf("# of installed rules: %d\n", metric_ruleInstallCount);
					printf("Avg. flow completion time: %.3lf\n", metric_avgFlowCompleteTime/totFlow);
					printf("Wireless:Wired = %d:%d\n", numberOfWirelessFlow, numberOfWiredFlow);
					printf("Replacement %d / %d / %d\n", ruleReplacementCore, ruleReplacementAggr, ruleReplacementEdge);
/*
					printf("%d %d %.3lf %d %d %d %d %d\n", metric_flowSetupRequest, metric_ruleInstallCount,
							metric_avgFlowCompleteTime/totFlow, numberOfWirelessFlow, numberOfWiredFlow,
							ruleReplacementCore, ruleReplacementAggr, ruleReplacementEdge);
*/
				}
				break;

			// Unknown
			case EVENT_UNKNOWN:
				printf("Error: unknown operation found.\n");
				break;
		}
	}
}
