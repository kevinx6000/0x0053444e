#include "fattree.h"
#include "../basic_lib.h"

// Wireless path
void Fattree::wirelessSP(void){

	// IDs
	int srcID, dstID, nowID;
	double xx[2], yy[2];

	// A* BFS
	int prev[pod*pod/2];
	int sft = numberOfCore + numberOfAggregate;
	priority_queue<myPair>pque;
	myPair now, nxt;
	memset(prev, 0, sizeof(prev));

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

		// All pair destination
		srcID = sft + i;
		for(int j = i+1; j < numberOfEdge; j++){
			dstID = sft + j;
			
			// A* BFS
			now.id = srcID;
			now.hop = 0;
			now.dis = 0.0;
			prev[srcID-sft] = srcID;
			pque.push(now);
			while(!pque.empty()){
				now = pque.top();
				pque.pop();
				for(int z = 0; z < sw[now.id]->wlink.size(); z++){
					nxt.id = sw[now.id]->wlink[z].id;
					if(!prev[nxt.id-sft]){
						prev[nxt.id-sft] = now.id;
						nxt.hop = now.hop+1;
						xx[0] = sw[nxt.id]->posXY[0];
						yy[0] = sw[nxt.id]->posXY[1];
						xx[1] = sw[dstID]->posXY[0];
						yy[1] = sw[dstID]->posXY[1];
						nxt.dis = myDis(xx[0], yy[0], xx[1], yy[1]);
						pque.push(nxt);
					}
					if(prev[dstID-sft]) break;
				}
				if(prev[dstID-sft]) break;
			}

			// Path
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
			memset(prev, 0, sizeof(prev));
			while(!pque.empty()) pque.pop();
		}
	}
}
