
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

// Controller
void Fattree::controller(Event ctrEvt){

	// Variables
	int nid;
	int pathLen;
	int nowFlowID;
	double delay;
	double compDelay = CONTROL_CPU_DELAY;
	double controlOutDelay = CONTROL_OUT_DELAY;
	double TCAMWriteDelay = TCAM_WRITE_DELAY;
	IP dstIP;
	Event evt, ret;
	Packet pkt;
	Entry ent;
	vector<Event>flowSetupEvent;
	vector<Entry>vent;

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

			// The rule is already installed into switch earlier
			else if(!alreadyInstall(evt)){
				fprintf(stderr, "< Controller ignore the request >\n");
				continue;
			}

			// Known flow, only install at this switch
			else if(rcdFlowID[pkt]){

				// Rule needed
				nowFlowID = rcdFlowID[pkt];
				ent = rule(nid, allEntry[nowFlowID]);

				// Switch side install rule
				ret.setEventType(EVENT_INSTALL);
				ret.setTimeStamp(ctrEvt.getTimeStamp() + compDelay + controlOutDelay + TCAMWriteDelay);
				ret.setID(nid);
				ret.setPacket(pkt);
				ret.setEntry(ent);
				eventQueue.push(ret);

				// Controller side copy
				if(copyTCAM[nid].size() >= maxEntry)
					copyTCAM[nid].erase(copyTCAM[nid].begin());
				copyTCAM[nid].push_back(ent);
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
	cumQue.clear();

	// Currently, all flow setup apply wired policy
	for(int j = 0; j < flowSetupEvent.size(); j++){
		evt = flowSetupEvent[j];
		nid = evt.getID();
		pkt = evt.getPacket();

		// Assign flow ID
		nowFlowID = flowIDCount ++;
		rcdFlowID[pkt] = nowFlowID;

		// Wired policy
		vent = wired(nid, pkt);

		// Install rule
		for(int i = 0; i < vent.size(); i++){

			// Switch side event
			ret.setEventType(EVENT_INSTALL);
			ret.setTimeStamp(ctrEvt.getTimeStamp() + compDelay + controlOutDelay + TCAMWriteDelay);
			ret.setID(vent[i].getSID());
			ret.setPacket(pkt);
			ret.setEntry(vent[i]);
			eventQueue.push(ret);

			// Controller side copy
			if(copyTCAM[vent[i].getSID()].size() >= maxEntry)
				copyTCAM[vent[i].getSID()].erase(copyTCAM[vent[i].getSID()].begin());
			copyTCAM[vent[i].getSID()].push_back(vent[i]);
		}

		// Record inserted entries
		allEntry.push_back(vent);

		// Clear Entry
		vent.clear();
	}

	// DEBUG: if no event handled, stop
	if(!eventQueue.size()) return;

	// The next timeout time
	evt = ctrEvt;
	evt.setEventType(EVENT_INTERVAL);
	evt.setTimeStamp(evt.getTimeStamp()+10.0);
	eventQueue.push(evt);	
	return;
}
