#pragma once
#include <vector>
#include <set>
#include <functional>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "utility/DiscreteDistribution.h"
#include "utility/PriorityQueue.h"
#include "utility/ResultContainer.h"
#include "decisionAlgorithms/DecisionAlgorithm.h"
#include "decisionAlgorithms/KBestBackup.h"
#include "decisionAlgorithms/ScalarBackup.h"
#include "expansionAlgorithms/ExpansionAlgorithm.h"
#include "expansionAlgorithms/AStar.h"
#include "expansionAlgorithms/BreadthFirst.h"
#include "expansionAlgorithms/DepthFirst.h"
#include "expansionAlgorithms/Risk.h"
#include "expansionAlgorithms/Confidence.h"
#include "learningAlgorithms/LearningAlgorithm.h"
#include "learningAlgorithms/Dijkstra.h"
#include "learningAlgorithms/Ignorance.h"

#include <time.h>

using namespace std;

template <class Domain>
class RealTimeSearch
{
public:
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;
    typedef typename Domain::HashState Hash;

    struct Node {
        Cost g;
        Cost h;
        Cost d;
        Cost derr;
        Cost epsH;
        Cost epsD;
        Node* parent;
        State stateRep;
        int owningTLA;
        bool open;
        int delayCntr;
        DiscreteDistribution distribution;

    public:
        Cost getGValue() const { return g; }
        Cost getHValue() const { return h; }
        Cost getDValue() const { return d; }
        Cost getDErrValue() const { return derr; }
        Cost getFValue() const { return g + h; }
        Cost getEpsilonH() const { return epsH; }
        Cost getEpsilonD() const { return epsD; }
        Cost getFHatValue() const { return g + getHHatValue(); }
        Cost getDHatValue() const { return (derr / (1.0 - epsD)); }
        Cost getHHatValue() const { return h + getDHatValue() * epsH; }
        State getState() const { return stateRep; }
        Node* getParent() const { return parent; }
        int getOwningTLA() const { return owningTLA; }

        void setHValue(Cost val) { h = val; }
        void setGValue(Cost val) { g = val; }
        void setDValue(Cost val) { d = val; }
        void setDErrValue(Cost val) { derr = val; }
        void setEpsilonH(Cost val) { epsH = val; }
        void setEpsilonD(Cost val) { epsD = val; }
        void setState(State s) { stateRep = s; }
        void setOwningTLA(int tla) { owningTLA = tla; }
        void setParent(Node* p) { parent = p; }

        bool onOpen() { return open; }
        void close() { open = false; }
        void reOpen() { open = true; }

        void incDelayCntr() { delayCntr++; }
        int getDelayCntr() { return delayCntr; }

        void markStart() { stateRep.markStart(); }

        Node(Cost g,
                Cost h,
                Cost d,
                Cost derr,
                Cost epsH,
                Cost epsD,
                State state,
                Node* parent,
                int tla)
                : g(g),
                  h(h),
                  d(d),
                  derr(derr),
                  epsH(epsH),
                  epsD(epsD),
                  stateRep(state),
                  parent(parent),
                  owningTLA(tla) {
            open = true;
            delayCntr = 0;
        }

        friend std::ostream& operator<<(std::ostream& stream,
                const Node& node) {
            stream << node.getState() << endl;
            stream << "f: " << node.getFValue() << endl;
            stream << "g: " << node.getGValue() << endl;
            stream << "h: " << node.getHValue() << endl;
            stream << "derr: " << node.getDErrValue() << endl;
            stream << "d: " << node.getDValue() << endl;
            stream << "epsilon-h: " << node.getEpsilonH() << endl;
            stream << "epsilon-d: " << node.getEpsilonD() << endl;
            stream << "f-hat: " << node.getFHatValue() << endl;
            stream << "d-hat: " << node.getDHatValue() << endl;
            stream << "h-hat: " << node.getHHatValue() << endl;
            stream << "action generated by: " << node.getState().getLabel()
                   << endl;
            stream << "-----------------------------------------------" << endl;
            stream << endl;
            return stream;
        }

        static bool compareNodesF(const Node* n1, const Node* n2) {
            // Tie break on g-value
            if (n1->getFValue() == n2->getFValue()) {
                return n1->getGValue() > n2->getGValue();
            }
            return n1->getFValue() < n2->getFValue();
        }

        static bool compareNodesFHat(const Node* n1, const Node* n2) {
            // Tie break on g-value
			if (n1->getFHatValue() == n2->getFHatValue())
			{
                if (n1->getFValue() == n2->getFValue())
                {
				    return n1->getGValue() > n2->getGValue();
                }
                return n1->getFValue() < n2->getFValue();
			}
			return n1->getFHatValue() < n2->getFHatValue();
        }

        static bool compareNodesH(const Node* n1, const Node* n2) {
            if (n1->getHValue() == n2->getHValue()) {
                return n1->getGValue() > n2->getGValue();
            }
            return n1->getHValue() < n2->getHValue();
        }
    };

    struct TopLevelAction {
    public:
        PriorityQueue<Node*> open;
        Cost expectedMinimumPathCost;
        Node* topLevelNode;
        vector<Node*> kBestNodes;
        DiscreteDistribution belief;

        TopLevelAction() { open.swapComparator(Node::compareNodesFHat); }

        TopLevelAction(const TopLevelAction& tla) {
            open = tla.open;
            expectedMinimumPathCost = tla.expectedMinimumPathCost;
            topLevelNode = tla.topLevelNode;
            kBestNodes = tla.kBestNodes;
            belief = tla.belief;
        }

        TopLevelAction& operator=(const TopLevelAction& rhs) {
            if (&rhs == this)
                return *this;
            open = rhs.open;
            expectedMinimumPathCost = rhs.expectedMinimumPathCost;
            topLevelNode = rhs.topLevelNode;
            kBestNodes = rhs.kBestNodes;
            belief = rhs.belief;
            return *this;
        }
    };

    RealTimeSearch(Domain& domain,
            string expansionModule,
            string learningModule,
            string decisionModule,
            double lookahead,
            double k = 1,
            string belief = "normal")
            : domain(domain),
              expansionPolicy(expansionModule),
              learningPolicy(learningModule),
              decisionPolicy(decisionModule),
              lookahead(lookahead) {
        if (expansionModule == "a-star") {
            expansionAlgo = new AStar<Domain, Node, TopLevelAction>(
                    domain, lookahead, "f");
        } else if (expansionModule == "f-hat") {
            expansionAlgo = new AStar<Domain, Node, TopLevelAction>(
                    domain, lookahead, "fhat");
        } else if (expansionModule == "dfs") {
            expansionAlgo = new DepthFirst<Domain, Node, TopLevelAction>(
                    domain, lookahead);
        } else if (expansionModule == "bfs") {
            expansionAlgo = new BreadthFirst<Domain, Node, TopLevelAction>(
                    domain, lookahead);
        } else if (expansionModule == "risk") {
            expansionAlgo = new Risk<Domain, Node, TopLevelAction>(
                    domain, lookahead, 1);
        }

        if (learningModule == "none") {
            learningAlgo = new Ignorance<Domain, Node, TopLevelAction>;
        } else if (learningModule == "learn") {
            learningAlgo = new Dijkstra<Domain, Node, TopLevelAction>(domain);
        }

        if (decisionModule == "minimin") {
            decisionAlgo =
                    new ScalarBackup<Domain, Node, TopLevelAction>("minimin");
        } else if (decisionModule == "bellman") {
            decisionAlgo =
                    new ScalarBackup<Domain, Node, TopLevelAction>("bellman");
        } else if (decisionModule == "k-best") {
            decisionAlgo = new KBestBackup<Domain, Node, TopLevelAction>(
                    domain, k, belief, lookahead);
        }
    }

    ~RealTimeSearch() { clean(); }

    ResultContainer search() {
        domain.initialize(expansionPolicy, lookahead);

        ResultContainer res;

        // Get the start node
        Node* start = new Node(0,
                domain.heuristic(domain.getStartState()),
                domain.distance(domain.getStartState()),
                domain.distanceErr(domain.getStartState()),
                domain.epsilonHGlobal(),
                domain.epsilonDGlobal(),
                domain.getStartState(),
                NULL,
                -1);

        clock_t startTime = clock();
        int iterationCounter = 1;

        while (1) {
            // Check if a goal has been reached
            if (domain.isGoal(start->getState())) {
                // Calculate path cost and return solution
                calculateCost(start, res);

                res.totalCpuTime = double(clock() - startTime) /
                        CLOCKS_PER_SEC / iterationCounter;

                return res;
            }

            restartLists(start);

            // Exploration Phase
            domain.updateEpsilons();

            // First, generate the top-level actions
            generateTopLevelActions(start, res);

            // Expand some nodes until expnasion limit
            expansionAlgo->expand(open, closed, tlas, duplicateDetection, res);

            // Check if this is a dead end 
			// or reach the lookahead limit
            if (open.empty()) {
                res.totalCpuTime = double(clock() - startTime) /
                        CLOCKS_PER_SEC / iterationCounter;
                break;
            }

            //  Learning Phase
            learningAlgo->learn(open, closed);

            // Decision-making Phase
            start = decisionAlgo->backup(open, tlas, start);

            iterationCounter++;
        }

        return res;
	}

	ResultContainer lastIncrementalDecision()
	{
		domain.initialize(expansionPolicy, lookahead);

		ResultContainer res;

		// Get the start node
		Node* start = new Node(0, domain.heuristic(domain.getStartState()), domain.distance(domain.getStartState()),
			domain.distanceErr(domain.getStartState()), domain.epsilonHGlobal(), domain.epsilonDGlobal(), domain.getStartState(), NULL, -1);

		// Check if a goal has been reached
		if (domain.isGoal(start->getState()))
		{
			// Calculate path cost and return solution
			calculateCost(start, res);

			return res;
		}

		restartLists(start);

		// Exploration Phase
		domain.updateEpsilons();
		// First, generate the top-level actions
		generateTopLevelActions(start, res);

		// Expand some nodes until expnasion limit
		expansionAlgo->expand(open, closed, tlas, duplicateDetection, res);

		//  Learning Phase
		learningAlgo->learn(open, closed);

		// Decision-making Phase
		start = decisionAlgo->backup(open, tlas, start);

		// Empty OPEN and CLOSED
		open.clear();

		// delete all of the nodes from the last expansion phase
		for (typename unordered_map<State, Node*, Hash>::iterator it = closed.begin(); it != closed.end(); it++)
			if (it->second != start)
				delete it->second;

		closed.clear();

		open.push(start);
		closed[start->getState()] = start;

		expansionAlgo->incrementLookahead();

		// Solve the search optimally
		expansionAlgo->expand(open, closed, tlas, duplicateDetection, res);

		open.swapComparator(Node::compareNodesF);
		start = open.top();

		if (domain.isGoal(start->getState()))
		{
			// Calculate path cost and return solution
			calculateCost(start, res);

			return res;
		}
	}

private:
	static bool duplicateDetection(Node* node, unordered_map<State, Node*, Hash>& closed, PriorityQueue<Node*>& open, 
		vector<TopLevelAction>& tlaList)
	{
		// Check if this state exists 
		typename unordered_map<State, Node*, Hash>::iterator it = closed.find(node->getState());

		if (it != closed.end())
		{
			// This state has been generated before, check if its node is on OPEN
			if (it->second->onOpen())
			{
				// This node is on OPEN, keep the better g-value
				if (node->getGValue() < it->second->getGValue())
				{
					tlaList[it->second->getOwningTLA()].open.remove(it->second);
					it->second->setGValue(node->getGValue());
					it->second->setParent(node->getParent());
					it->second->setHValue(node->getHValue());
					it->second->setDValue(node->getDValue());
					it->second->setDErrValue(node->getDErrValue());
					it->second->setEpsilonH(node->getEpsilonH());
					it->second->setEpsilonD(node->getEpsilonD());
					it->second->setState(node->getState());
					it->second->setOwningTLA(node->getOwningTLA());
					tlaList[node->getOwningTLA()].open.push(it->second);
				}
			}
			else
			{
				// This node is on CLOSED, compare the f-values. If this new f-value is better, reset g, h, and d. 
				// Then reopen the node.
				if (node->getFValue() < it->second->getFValue())
				{
					it->second->setGValue(node->getGValue());
					it->second->setParent(node->getParent());
					it->second->setHValue(node->getHValue());
					it->second->setDValue(node->getDValue());
					it->second->setDErrValue(node->getDErrValue());
					it->second->setEpsilonH(node->getEpsilonH());
					it->second->setEpsilonD(node->getEpsilonD());
					it->second->setState(node->getState());
					it->second->setOwningTLA(node->getOwningTLA());
					tlaList[node->getOwningTLA()].open.push(it->second);
					it->second->reOpen();
					open.push(it->second);
				}
			}

			return true;
		}

		return false;
	}

	void generateTopLevelActions(Node* start, ResultContainer& res)
	{
		// The first node to be expanded in any problem is the start node
		// Doing so yields the top level actions
		start->close();
		closed[start->getState()] = start;
		res.nodesExpanded++;

		vector<State> children = domain.successors(start->getState());
		res.nodesGenerated += children.size();

		State bestChild;
		Cost bestF = numeric_limits<double>::infinity();

		for (State child : children)
		{
			Node* childNode = new Node(start->getGValue() + domain.getEdgeCost(child),
				domain.heuristic(child), domain.distance(child), domain.distanceErr(child), domain.epsilonHGlobal(),
				domain.epsilonDGlobal(), child, start, tlas.size());

			if (childNode->getFValue() < bestF)
			{
				bestF = childNode->getFValue();
				bestChild = child;
			}

			// No top level action will ever be a duplicate, so no need to check.
			// Make a new top level action and push this node onto its open
			TopLevelAction tla;
			tla.topLevelNode = childNode;

			childNode->distribution = DiscreteDistribution(5, childNode->getFValue(), childNode->getFHatValue(), 
				childNode->getDValue(), childNode->getFHatValue() - childNode->getFValue());

			tla.expectedMinimumPathCost = childNode->distribution.expectedCost();

			// Push this node onto open and closed
			closed[child] = childNode;
			open.push(childNode);
			tla.open.push(childNode);

			// Add this top level action to the list
			tlas.push_back(tla);
		}

		// Learn one-step error
		if (!children.empty())
		{
			Cost epsD = (1 + domain.distance(bestChild)) - start->getDValue();
			Cost epsH = (domain.getEdgeCost(bestChild) + domain.heuristic(bestChild)) - start->getHValue();

			domain.pushEpsilonHGlobal(epsH);
			domain.pushEpsilonDGlobal(epsD);
		}
	}

	void restartLists(Node* start)
	{
		// clear the TLA list
		tlas.clear();

		// Empty OPEN and CLOSED
		open.clear();

		// delete all of the nodes from the last expansion phase
		for (typename unordered_map<State, Node*, Hash>::iterator it = closed.begin(); it != closed.end(); it++)
			if (it->second != start)
				delete it->second;

		closed.clear();
	}

	void clean()
	{
		// clear the TLA list
		tlas.clear();

		// Empty OPEN and CLOSED
		open.clear();

		// delete all of the nodes from the last expansion phase
		for (typename unordered_map<State, Node*, Hash>::iterator it = closed.begin(); it != closed.end(); it++)
			delete it->second;

		closed.clear();

		delete expansionAlgo;
		delete learningAlgo;
		delete decisionAlgo;
	}

	void calculateCost(Node* solution, ResultContainer& res)
	{
		res.solutionFound = true;
		res.solutionCost = solution->getFValue();
	}

protected:
	Domain& domain;
	ExpansionAlgorithm<Domain, Node, TopLevelAction>* expansionAlgo;
	LearningAlgorithm<Domain, Node, TopLevelAction>* learningAlgo;
	DecisionAlgorithm<Domain, Node, TopLevelAction>* decisionAlgo;
	PriorityQueue<Node*> open;
	unordered_map<State, Node*, Hash> closed;
	vector<TopLevelAction> tlas;

	double lookahead;
	string expansionPolicy;
	string learningPolicy;
	string decisionPolicy;
};
