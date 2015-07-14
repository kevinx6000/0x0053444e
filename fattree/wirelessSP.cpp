#include "fattree.h"
#include "../basic_lib.h"

// Wireless path
void Fattree::wirelessSP(void){

	// IDs
	int srcID, dstID, nowID, nxtID;

	// BFS
	queue<int>BFS;
	map<int,int>prev;

	// Paths
	vector<int>rev;
	vector<int>path;
	vector< vector<int> > rowPath;

	// Empty path for all starting node
	for(int i = 0; i < numberOfEdge; i++)
		this->wlPath.push_back(rowPath);

	// All pair shortest path
	for(int i = 0; i < numberOfEdge; i++){

		// Self: empty path
		this->wlPath[i].push_back(path);

		// BFS
		srcID = numberOfCore + numberOfAggregate + i;
		BFS.push(srcID);
		prev[srcID] = srcID;
		while(!BFS.empty()){
			nowID = BFS.front();
			BFS.pop();
			for(int j = 0; j < sw[nowID]->wlink.size(); j++){
				nxtID = sw[nowID]->wlink[j].id;
				if(!prev[nxtID]){
					prev[nxtID] = nowID;
					BFS.push(nxtID);
				}
			}
		}

		// All destination
		for(int j = i+1; j < numberOfEdge; j++){
			dstID = numberOfCore + numberOfAggregate + j;
			nowID = dstID;
			while(nowID != srcID){
				rev.push_back(nowID);
				nowID = prev[nowID];
			}
			rev.push_back(srcID);
			for(int z = rev.size()-1; z >= 0; z--)
				path.push_back(rev[z]);
			
			// Row path
			this->wlPath[i].push_back(path);
			this->wlPath[j].push_back(rev);
			
			// Clear path
			path.clear();
			rev.clear();
		}

		// Clear prev
		prev.clear();
	}

// DEBUG
fprintf(stderr, "START DEBUGGING:\n");
while(scanf("%d%d", &srcID, &dstID)==2){
	srcID -= (numberOfCore + numberOfAggregate);
	dstID -= (numberOfCore + numberOfAggregate);
	if(!this->wlPath[srcID][dstID].size()) printf("Empty\n");
	else{
		for(int i = 0; i < this->wlPath[srcID][dstID].size(); i++)
			printf(" %d", this->wlPath[srcID][dstID][i]);
		printf("\n");
	}
}
}
