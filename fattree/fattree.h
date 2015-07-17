
// Headers
#include "../basic_lib.h"
#include "../node/node.h"
#include "../event/event.h"
#include "../switch/switch.h"
#include "../packet/packet.h"

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
		int getNumberOfPod(void);		// Get number of Pods
		Node **getNodePtr(void);		// Get node pointer

		// Public data
		vector< vector< vector<int> > > wlPath;	// Wireless paths

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
		vector<Event>cumQue;			// Cumulated event queue

		// Private method
		void controller(Event);			// Handles a batch of flow setup requests
		void install(Event);			// Install rules into switch
		Entry rule(int,vector<Entry>);	// Extract rule from flow path
		void cumulate(Event);			// Cumulate events until timeout
		bool legalAddr(IP);				// Check if address is legal
		bool alreadyInstall(Event);		// Check if flow setup is needed
		void modifyCap(Packet,double);	// Modify capacity used along the path
		void wirelessSP(void);			// Pre-process wireless shortest path
		int pathInit(Packet,map<int,int>&);		// Initialize the prev array with -1
		bool wired(int,Packet,vector<Entry>&,int);		// Wired policy
		bool wireless(int,Packet,vector<Entry>&,int);	// Wireless policy
		double vecdot(double[],double[],double[],double[]);	// Calculate vector dot
		double vecdis(double[],double[],double[],double[]);	// Calculate vector distance

		
};
#endif
