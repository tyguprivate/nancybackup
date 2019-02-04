#include "domain/TreeWorld.h"
#include "domain/SlidingTilePuzzle.h"

using namespace std;

template <class Domain>
class RandomSearch
{
public:
	typedef typename Domain::State State;
	typedef typename Domain::Cost Cost;

	RandomSearch(Domain& domain, double lookahead)
	:domain(domain), lookahead(lookahead){
	}

	double randomSearch(){
		domain.initialize("randomSearch", lookahead);
		State cur = domain.getStartState();
		int r;
		double cost = 0;
		srand(time(0));

		while (!domain.isGoal(cur)){
			vector<State> children = domain.successors(cur);

			r = randNum(children.size());
			cur = children[r];
			cost += domain.getEdgeCost(cur);
		}
		return cost;
	}

	double randomSampling(int ksamples){
		domain.initialize("randomSampling", lookahead);
		State cur = domain.getStartState();
		int r;
		double cost = 0;
		srand(time(0));

	}

	double getSample(State s){
		State cur = s;
		double cost = 0;
		int start_depth = s.getDepth();
		while(s.getDepth() - start_depth < lookahead){
			vector<State> children = domain.successors(cur);
			int r = randNum(children.size());
			cur = children[r];
			cost += domain.getEdgeCost(cur);
		}
		return cost;
	}

private:
	static int randNum(int max){
		int x = rand() % max;
		return x;
	}

protected:
	Domain& domain;
	double lookahead;
};