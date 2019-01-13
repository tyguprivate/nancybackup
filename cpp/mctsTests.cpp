#include <iostream>
#include <string>
#include <fstream>
#include "RealTimeSearch.h"
#include "decisionAlgorithms/ScalarBackup.h"
#include "decisionAlgorithms/KBestBackup.h"
#include "expansionAlgorithms/AStar.h"
#include "learningAlgorithms/Ignorance.h"
#include "learningAlgorithms/Dijkstra.h"
#include "domain/TreeWorld.h"
#include "domain/SlidingTilePuzzle.h"

using namespace std;

static int randNum(int max){
	int x = rand() % max;
	return x;
}

double MCTS(TreeWorld domain, int lookaheadDepth){
		domain.initialize("mcts", lookaheadDepth);
		TreeWorld::State cur = domain.getStartState();
		int r;
		double cost = 0;
		srand(time(0));

		while (!domain.isGoal(cur)){
			vector<TreeWorld::State> children = domain.successors(cur);

			r = randNum(children.size());
			cur = children[r];
			cost += domain.getEdgeCost(cur);
		}
		return cost;
}

int main(int argc, char** argv)
{

	
	if (argc > 4 || argc < 3)
	{
		cout << "Wrong number of arguments: ./mctsTests <Domain Type> <lookahead depth> <optional: output file> < <domain file>" << endl;
		cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
		exit(1);
	}

	// Get the lookahead depth
	int lookaheadDepth = stoi(argv[2]);

	// Get the domain type
	string domain = argv[1];

	ResultContainer mctsRes;
	string result;
	if (domain == "TreeWorld") {

		// Make a tree world
		TreeWorld world = TreeWorld(cin);	
		result = "{ \"Random\": " + to_string(MCTS(world, lookaheadDepth)) + " }";	

	} else if (domain == "SlidingPuzzle") {
        cout << "Needs to be implemented" << endl;
		exit(1);
	} else {
		cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
		exit(1);
	}
    
	if (argc < 4)
	{
		cout << result << endl;
	}
	else
	{
		ofstream out(argv[3]);

		out << result;
		out.close();
	}
}
