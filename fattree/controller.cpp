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
#include "../mysort/mysort.h"

// Controller
void Fattree::controller(Event ctrEvt){

	// Variables
	int nid;
	int pathLen;
	int nowFlowID;
	int temp;
	double delay;
	double flowSetupDelay = FLOW_SETUP_DELAY;
	double computePathDelay = CONTROL_PATH_DELAY;
	Event evt, ret;
	Packet pkt;
	Entry ent;
	vector<Event>flowSetupEvent;
	vector<Entry>vent;
	bool hasHandle = false;

	// Classify events
	for(int i = 0; i < cumQue.size(); i++){
		evt = cumQue[i];
		nid = evt.getID();
		pkt = evt.getPacket();

		// Flow Setup Request
		if(evt.getEventType() == EVENT_FLOWSETUP){

			// Illegal destination address
			if(!legalAddr(pkt.getDstIP())){
				delay = 1.0;
				ret.setTimeStamp(ctrEvt.getTimeStamp() + delay);
				ret.setEventType(EVENT_UNKNOWN);
				this->eventQueue.push(ret);
				continue;
			}

			// Known flow
			else if(rcdFlowID[pkt]){
				nowFlowID = rcdFlowID[pkt];
				vent.clear();

				// Wireless policy or not at starting edge switch
				if(allEntry[nowFlowID][0].isWireless() || nid != allEntry[nowFlowID][0].getSID() || !wireless(nid, pkt, vent, temp)){

					// Rule needed
					if(rule(nid, allEntry[nowFlowID], ent)){
						ent.setExpire(ctrEvt.getTimeStamp() + flowSetupDelay + ENTRY_EXPIRE_TIME);

						// Switch side install rule
						ret.setEventType(EVENT_INSTALL);
						ret.setTimeStamp(ctrEvt.getTimeStamp() + flowSetupDelay);
						ret.setID(nid);
						ret.setPacket(pkt);
						ret.setEntry(ent);
						eventQueue.push(ret);
					}
					// Rule not found
					else{
						/* This may cause by rule deletion of starting edge policy.*/
						fprintf(stderr, "Rule extract failed!!!\n");
					}
				}

				// Wired policy @ starting edge switch, and has wireless solution
				else{

					/* Remove entries along the path for this flowID */
					for(int j = 0; j < allEntry[nowFlowID].size(); j++){
						temp = allEntry[nowFlowID][j].getSID();
						for(int k = 0; k < sw[temp]->TCAM.size(); k++)
							if(sw[temp]->TCAM[k].isMatch(pkt)){
								sw[temp]->TCAM.erase(sw[temp]->TCAM.begin()+k);
								k--;
							}
					}

					// Install rule
					for(int j = 0; j < vent.size(); j++){

						// Switch side event
						ret.setEventType(EVENT_INSTALL);
						ret.setTimeStamp(ctrEvt.getTimeStamp() + flowSetupDelay + computePathDelay);
						ret.setID(vent[j].getSID());
						ret.setPacket(pkt);
						ret.setEntry(vent[j]);
						eventQueue.push(ret);
					}

					/* Update allEntry */
					allEntry[nowFlowID] = vent;
				}
			}

			// Require to setup along the path
			else flowSetupEvent.push_back(evt);
		}

		// Unknown events
		else{
			delay = 1;
			ret.setTimeStamp(ctrEvt.getTimeStamp() + delay);
			ret.setEventType(EVENT_UNKNOWN);
			eventQueue.push(ret);
		}
	}
	if(((int)cumQue.size()) > 0) hasHandle = true;
	cumQue.clear();

	// Sort with the largest gap between wired & wireless
	mySort msrt(this);
	sort(flowSetupEvent.begin(), flowSetupEvent.end(), msrt);

	// Currently, all flow setup apply wired policy
	for(int j = 0; j < flowSetupEvent.size(); j++){
		evt = flowSetupEvent[j];
		nid = evt.getID();
		pkt = evt.getPacket();
	
		// Assign flow ID
		nowFlowID = flowIDCount ++;
		rcdFlowID[pkt] = nowFlowID;

		// Wirless policy first, then wired policy
		temp = ctrEvt.getTimeStamp() + flowSetupDelay + computePathDelay;
		if(wireless(nid, pkt, vent, temp) || wired(nid, pkt, vent, temp)){

			// Install rule
			for(int i = 0; i < vent.size(); i++){

				// Switch side event
				ret.setEventType(EVENT_INSTALL);
				ret.setTimeStamp(ctrEvt.getTimeStamp() + flowSetupDelay + computePathDelay);
				ret.setID(vent[i].getSID());
				ret.setPacket(pkt);
				ret.setEntry(vent[i]);
				eventQueue.push(ret);
			}
			
			// Record inserted entries
			allEntry.push_back(vent);
		}

		// No such path exists
		else{
			fprintf(stderr, "Error: %s to %s: ", pkt.getSrcIP().fullIP.c_str(), pkt.getDstIP().fullIP.c_str());
			fprintf(stderr, "No such path exists.\n");
			/* Here we may need to handle such situation */
		}

		// Clear Entry
		vent.clear();
	}

	// DEBUG: if no event handled, stop
	if(!eventQueue.size()) return;

	// DEBUG log
	if(hasHandle) printf("[%6.1lf] Controller: Waiting for next handle...\n", ctrEvt.getTimeStamp());

	// The next timeout time
	evt = ctrEvt;
	evt.setEventType(EVENT_INTERVAL);
	evt.setTimeStamp(evt.getTimeStamp()+CONTROL_BATCH);
	eventQueue.push(evt);
	return;
}
