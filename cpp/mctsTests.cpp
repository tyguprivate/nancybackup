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




int main(int argc, char** argv)
{
	cout << "starting" << endl;

	
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

	if (domain == "TreeWorld") {
		cout << "testing on TreeWorld" << endl;

		// Make a tree world
		TreeWorld world = TreeWorld(cin);

		// Run DFS with differing backup methods for decision making
		
		RealTimeSearch<TreeWorld> mcts(world, "mcts", "none", "mcts", lookaheadDepth);
		mctsRes = mcts.search();



	} else if (domain == "SlidingPuzzle") {
        cout << "Needs to be implemented" << endl;
		exit(1);
	} else {
		cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
		exit(1);
	}

    /*
	string result = "{ \"Minimin\": " + to_string(miniminRes.solutionCost) + ", \"Bellman\": " +
		to_string(bellmanRes.solutionCost) + ", \"Nancy\": " + to_string(nancyRes.solutionCost) +
		", \"Cserna\": " + to_string(csernaRes.solutionCost) + ", \"Cserna Pemberton Belief\": " +
		to_string(pembertonRes.solutionCost) + ", \"K-Best 3\": " + to_string(k3Res.solutionCost) +
		", \"K-Best 10\": " + to_string(k10Res.solutionCost) + ", \"K-Best 30\": " + to_string(k30Res.solutionCost) +
		", \"K-Best 3 Pemberton Belief\": " + to_string(pk3Res.solutionCost) +
		", \"K-Best 10 Pemberton Belief\": " + to_string(pk10Res.solutionCost) +
		", \"K-Best 30 Pemberton Belief\": " + to_string(pk30Res.solutionCost) +
		", \"Lookahead\": " + to_string(lookaheadDepth) + " }";
    */

	if (argc < 4)
	{
		cout << "result" << endl;
	}
	else
	{
		ofstream out(argv[3]);

		out << "result";
		out.close();
	}

}
