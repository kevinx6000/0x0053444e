
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
	int sid, siz, arrive, totFlow;
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
	int interfereCnt = 0;
	int nowID, nxtID;
	int i;
	int FUCK;
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

					// Record number of flow for statistic info
					/* At starting edge only */
					if(evt.getID() < numberOfCore + numberOfAggregate + numberOfEdge){
						nowFlowID = rcdFlowID[evt.getPacket()];
						if(evt.getID() == allEntry[nowFlowID][0].getSID()){

							// Up to aggr or core only
							if(evt.getPacket().getSrcIP().byte[1] != evt.getPacket().getDstIP().byte[1]
									|| evt.getPacket().getSrcIP().byte[2] != evt.getPacket().getDstIP().byte[2]){
								if(allEntry[nowFlowID][0].isWireless()){
									numberOfWirelessFlow ++;
									
									// Record interfere information
									i = 0;
FUCK = 0;
									nowID = allEntry[nowFlowID][0].getSID();
fprintf(stderr, "%d to ", nowID);
									nxtID = sw[nowID]->wlink[ allEntry[nowFlowID][0].getOutputPort() ].id;
									while(nxtID < numberOfCore + numberOfAggregate + numberOfEdge){
										interfereCnt += ((int)sw[nowID]->iList[ allEntry[nowFlowID][i].getOutputPort() ].size());
FUCK += ((int)sw[nowID]->iList[ allEntry[nowFlowID][i].getOutputPort() ].size());
//fprintf(stderr, "(");
//for(int j = 0; j < sw[nowID]->iList[ allEntry[nowFlowID][i].getOutputPort() ].size(); j++)
//	fprintf(stderr, " %d", sw[nowID]->iList[ allEntry[nowFlowID][i].getOutputPort() ][j]);
//fprintf(stderr, ")\n");
										nowID = nxtID;
										for(i = 0; i < allEntry[nowFlowID].size(); i++)
											if(allEntry[nowFlowID][i].getSID() == nowID) break;

										// BUG
										if(i == allEntry[nowFlowID].size()){
											fprintf(stderr, "SOME BUGS HERE\n");
											break;
										}
										if(!allEntry[nowFlowID][i].isWireless()){
fprintf(stderr, "%d: %d\n", nowID, FUCK);
											break;
										}
										nxtID = sw[nowID]->wlink[ allEntry[nowFlowID][i].getOutputPort() ].id;
									}
								}
								else numberOfWiredFlow ++;
							}
						}
					}

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
				metric_flowSetupRequest ++;
				break;

			// Interval timeout: handle batch of flow setup requests
			case EVENT_INTERVAL:
				controller(evt);
				break;

			// Install & forward
			case EVENT_INSTALL:
printf("[%6.1lf] Install: %d at %d\n", evt.getTimeStamp(), evt.getPacket().getSequence(), evt.getID());
				install(evt);
				metric_ruleInstallCount ++;

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
printf("[%6.1lf] %d flows arrives\n", evt.getTimeStamp(), arrive);

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
					printf("Interfere nodes = %d\n", interfereCnt);
				}
				break;

			// Unknown
			case EVENT_UNKNOWN:
				printf("Error: unknown operation found.\n");
				break;
		}
	}
}
