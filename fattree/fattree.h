
// Headers
#include "../basic_lib.h"
#include "../node/node.h"
#include "../event/event.h"
#include "../switch/switch.h"

// Fat Tree class
#ifndef FATTREE_H
#define FATTREE_H
class Fattree{
	public:
		// Public method
		Fattree(int);					// Constructor
		void readInput(void);			// Read the input packets 	
		void start(void);				// Start simulation
		int getNumberOfNode(void);		// Get number of nodes
		int getNumberOfCore(void);		// Get number of core switches
		int getNumberOfAggregate(void);	// Get number of aggregate switches
		int getNumberOfEdge(void);		// Get number of edge switchs
		int getNumberOfHost(void);		// Get number of hosts
	private:
		// Private data
		int pod;						// Number of pods
		int totalNode;					// Number of nodes
		int numberOfCore;				// Number of core switches
		int numberOfAggregate;			// Number of aggregate switches
		int numberOfEdge;				// Number of edge switches
		int numberOfHost;				// Number of hosts
		int maxEntry;					// Maximum number of TCAM entries
		int flowIDCount;				// Current flow ID count
		double compAvail;				// Controller available time (not used)
		Node **node;					// All nodes
		Switch **sw;					// All switches
		priority_queue<Event>eventQueue;	// Event queue
		map<Packet,int>rcdFlowID;			// Flow ID of a packet
		vector< vector<Entry> > allEntry;	// Flow entries of some flows
		vector< vector<Entry> > copyTCAM;	// TCAM at controller side
		vector<Event>cumQue;			// Cumulated event queue

		// Private method
		void controller(Event);			// Handles a batch of flow setup requests
		void install(Event);			// Install rules into switch
		Entry rule(int,vector<Entry>);	// Extract rule from flow path
		void cumulate(Event);			// Cumulate events until timeout
		bool legalAddr(IP);				// Check if address is legal
		bool alreadyInstall(Event);		// Check if flow setup is needed
		vector<Entry> wired(int,Packet);	// Wired policy
};
#endif
