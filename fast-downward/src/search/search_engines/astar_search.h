#ifndef SEARCH_ENGINES_ASTAR_SEARCH_H
#define SEARCH_ENGINES_ASTAR_SEARCH_H

#include "../global_operator.h"
#include "../search_engine.h"
#include "../utils/countdown_timer.h"

#include <queue>
#include <unordered_map>
#include <vector>

namespace options {
class Options;
}

struct AStarSearchNode {
    AStarSearchNode(
        const GlobalState state, int g, int h,
        AStarSearchNode *predecessor = nullptr,
        const GlobalOperator *achiever = nullptr)
        : state(state), g(g), h(h),
          predecessor(predecessor), achiever(achiever) {
    }

    GlobalState state;
    int g;
    int h;
    AStarSearchNode *predecessor;
    const GlobalOperator *achiever;

    struct Compare {
        bool operator()(const AStarSearchNode *lhs,
                        const AStarSearchNode *rhs) const {
            if((lhs->g + lhs->h) > (rhs->g + rhs->h)){
                return true;
            }else if((lhs->g + lhs->h) == (rhs->g + rhs->h) && lhs->h > rhs->h){
			return true;
		   } return false;
        }
    };
};

struct OpenList {
    explicit OpenList(utils::CountdownTimer &timer) :
        timer(timer) {}

    std::priority_queue<AStarSearchNode *, std::vector<AStarSearchNode *>,
                        AStarSearchNode::Compare> open_list;
    utils::CountdownTimer &timer;

    void insert(AStarSearchNode *node) {
        // insert your code here
        open_list.push(node);
    }

    AStarSearchNode *pop_min() {
    AStarSearchNode *a = open_list.top();
	open_list.pop();
	return a;
        // insert your code here
    }

    bool empty() const {
        // Usually, this would not be here, but it allows us to handle
        // all timer related methods outside of AStarSearch::search()
        if (timer.is_expired()) {
            std::cout << "Time limit reached. Abort search." << std::endl;
            return true;
        }
        return open_list.empty();
    }
};

struct ClosedList {
    std::unordered_map<StateID, int> distances;

    bool contains(const GlobalState &state) const {
        return distances.find(state.get_id()) != distances.end();
    }

    int& operator[](const GlobalState &state) {
        return distances[state.get_id()];
    }
};

namespace astar_search {
class AStarSearch : public SearchEngine {
    static const int INFTY = std::numeric_limits<int>::max();
    static const int NONE = -1;

    utils::CountdownTimer timer;

    OpenList open;
    ClosedList distances;
    Heuristic *heuristic;

    int evaluated_states;
    int expanded_states;
    int dead_ends;
    int best_h;

    const GlobalState &get_initial_state();
    std::vector<std::pair<const GlobalOperator *, GlobalState>>
        get_successors(const GlobalState &s);

    AStarSearchNode *make_node(AStarSearchNode *n, const GlobalOperator *op,
                               const GlobalState &s);
    AStarSearchNode *make_root_node();

    int h(const GlobalState &s);
    int g(AStarSearchNode *node) {
        return node->g;
    }
    bool is_goal(const GlobalState &s) {
        return test_goal(s);
    }
    void extract_solution(AStarSearchNode *node);

protected:
    virtual void search() override;

public:
    explicit AStarSearch(const options::Options &opts);
    virtual ~AStarSearch() = default;

    virtual void print_statistics() const override;
};
}

#endif
