#include "fattree.h"
#include "../basic_lib.h"

// Wireless path
void Fattree::wirelessSP(void){

	// IDs
	int srcID, dstID, nowID, nxtID;

	// BFS sequentially
	int prev[pod*pod/2];
	int sft = numberOfCore + numberOfAggregate;
	queue<int>BFS;

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

		// Clear previous record
		memset(prev, 0, sizeof(prev));

		// BFS
		srcID = sft + i;
		BFS.push(srcID);
		prev[srcID-sft] = srcID;
		while(!BFS.empty()){
			nowID = BFS.front();
			BFS.pop();
			for(int j = 0; j < sw[nowID]->wlink.size(); j++){

				// Sequentially pick one port
				nxtID = sw[nowID]->wlink[j].id;
				if(!prev[nxtID-sft]){
					prev[nxtID-sft] = nowID;
					BFS.push(nxtID);
				}
			}
		}

		// All destination Path
		for(int j = i+1; j < numberOfEdge; j++){
			dstID = sft + j;
			nowID = dstID;
			while(nowID != srcID){
				rev.push_back(nowID);
				nowID = prev[nowID-sft];
			}
			rev.push_back(srcID);
			for(int z = rev.size()-1; z >= 0; z--)
				path.push_back(rev[z]);

			// Row path
			this->wlPath[i].push_back(path);
			this->wlPath[j].push_back(rev);

			// Clear
			path.clear();
			rev.clear();
		}
	}
}
