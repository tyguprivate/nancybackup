#include <iostream>
#include <string>
#include <fstream>
#include "RealTimeSearch.h"
#include "decisionAlgorithms/ScalarBackup.h"
#include "decisionAlgorithms/KBestBackup.h"
#include "expansionAlgorithms/DepthFirst.h"
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
		cout << "Wrong number of arguments: ./backupTestsDFS <Domain Type> <lookahead depth> <optional: output file> < <domain file>" << endl;
		cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
		exit(1);
	}

	// Get the lookahead depth
	int lookaheadDepth = stoi(argv[2]);

	// Get the domain type
	string domain = argv[1];

	ResultContainer miniminRes;
	ResultContainer bellmanRes;
	ResultContainer pembertonRes;
	ResultContainer nancyRes;
	ResultContainer csernaRes;
	ResultContainer k3Res;
	ResultContainer k10Res;
	ResultContainer k30Res;
	ResultContainer pk3Res;
	ResultContainer pk10Res;
	ResultContainer pk30Res;

	double randomCost;

	if (domain == "TreeWorld")
	{
		// Make a tree world
		TreeWorld world = TreeWorld(cin);

		// Run DFS with differing backup methods for decision making	
		RealTimeSearch<TreeWorld> bellman(world, "dfs", "none", "bellman", lookaheadDepth);
		RealTimeSearch<TreeWorld> minimin(world, "dfs", "none", "minimin", lookaheadDepth);
		RealTimeSearch<TreeWorld> nancy(world, "dfs", "none", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<TreeWorld> cserna(world, "dfs", "none", "k-best", lookaheadDepth, numeric_limits<double>::infinity(), "normal");
		RealTimeSearch<TreeWorld> pemberton(world, "dfs", "none", "k-best", lookaheadDepth, numeric_limits<double>::infinity(), "pemberton");
		RealTimeSearch<TreeWorld> k3(world, "dfs", "none", "k-best", lookaheadDepth, 3, "normal");
		RealTimeSearch<TreeWorld> k10(world, "dfs", "none", "k-best", lookaheadDepth, 10, "normal");
		RealTimeSearch<TreeWorld> k30(world, "dfs", "none", "k-best", lookaheadDepth, 30, "normal");
		RealTimeSearch<TreeWorld> pk3(world, "dfs", "none", "k-best", lookaheadDepth, 3, "pemberton");
		RealTimeSearch<TreeWorld> pk10(world, "dfs", "none", "k-best", lookaheadDepth, 10, "pemberton");
		RealTimeSearch<TreeWorld> pk30(world, "dfs", "none", "k-best", lookaheadDepth, 30, "pemberton");

		miniminRes = minimin.search();
		bellmanRes = bellman.search();
		pembertonRes = pemberton.search();
		nancyRes = nancy.search();
		csernaRes = cserna.search();
		k3Res = k3.search();
		k10Res = k10.search();
		k30Res = k30.search();
		pk3Res = pk3.search();
		pk10Res = pk10.search();
		pk30Res = pk30.search();

		randomCost = MCTS(world, lookaheadDepth);
	}
	else if (domain == "SlidingPuzzle")
	{
		// Make a tile puzzle
		SlidingTilePuzzle world = SlidingTilePuzzle(cin);

		// Run DFS with differing backup methods for decision making
		RealTimeSearch<SlidingTilePuzzle> bellman(world, "dfs", "learn", "bellman", lookaheadDepth);
		RealTimeSearch<SlidingTilePuzzle> minimin(world, "dfs", "learn", "minimin", lookaheadDepth);
		RealTimeSearch<SlidingTilePuzzle> nancy(world, "dfs", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<SlidingTilePuzzle> cserna(world, "dfs", "learn", "k-best", lookaheadDepth, numeric_limits<double>::infinity(), "normal");
		RealTimeSearch<SlidingTilePuzzle> k3(world, "dfs", "learn", "k-best", lookaheadDepth, 3, "normal");
		RealTimeSearch<SlidingTilePuzzle> k10(world, "dfs", "learn", "k-best", lookaheadDepth, 10, "normal");
		RealTimeSearch<SlidingTilePuzzle> k30(world, "dfs", "learn", "k-best", lookaheadDepth, 30, "normal");

		miniminRes = minimin.search();
		bellmanRes = bellman.search();
		nancyRes = nancy.search();
		csernaRes = cserna.search();
		k3Res = k3.search();
		k10Res = k10.search();
		k30Res = k30.search();
	}
	else
	{
		cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
		exit(1);
	}

	string result = "{ \"Minimin\": " + to_string(miniminRes.solutionCost) + ", \"Bellman\": " +
		to_string(bellmanRes.solutionCost) + ", \"Nancy\": " + to_string(nancyRes.solutionCost) +
		", \"Cserna\": " + to_string(csernaRes.solutionCost) + ", \"Cserna Pemberton Belief\": " +
		to_string(pembertonRes.solutionCost) + ", \"K-Best 3\": " + to_string(k3Res.solutionCost) +
		", \"K-Best 10\": " + to_string(k10Res.solutionCost) + ", \"K-Best 30\": " + to_string(k30Res.solutionCost) + 
		", \"K-Best 3 Pemberton Belief\": " + to_string(pk3Res.solutionCost) +
		", \"K-Best 10 Pemberton Belief\": " + to_string(pk10Res.solutionCost) + 
		", \"K-Best 30 Pemberton Belief\": " + to_string(pk30Res.solutionCost) +
		", \"Random\": " + to_string(randomCost) +
		", \"Lookahead\": " + to_string(lookaheadDepth) + " }";

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
