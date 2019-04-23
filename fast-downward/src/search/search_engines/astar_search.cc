#include "astar_search.h"

#include "../heuristic.h"
#include "../heuristics/lm_cut_heuristic.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../task_utils/successor_generator.h"

#include <cassert>
#include <cstdlib>
#include <memory>

using namespace std;

namespace astar_search {

AStarSearch::AStarSearch(const Options &opts)
    : SearchEngine(opts),
      timer(60),
      open(timer),
      heuristic(opts.get<Heuristic *>("h")),
      evaluated_states(0),
      expanded_states(0),
      dead_ends(0),
      best_h(INFTY) {
}

void AStarSearch::print_statistics() const {
    cout << "Evaluated states: " << evaluated_states << " state(s)" << endl;
    cout << "Expanded states: " << expanded_states << " state(s)" << endl;
    cout << "Deadend states: " << dead_ends << " state(s)" << endl;
}

const GlobalState &AStarSearch::get_initial_state() {
    const GlobalState &initial_state = state_registry.get_initial_state();
    heuristic->notify_initial_state(initial_state);
    return initial_state;
}

AStarSearchNode *AStarSearch::make_root_node() {
    ++evaluated_states;
    const GlobalState &initial_state = get_initial_state();
    int h_value = h(initial_state);
    cout << "Initial h value: " << h_value << endl;
    return new AStarSearchNode(initial_state, 0, h_value);
}

void AStarSearch::extract_solution(AStarSearchNode *node) {
    assert(test_goal(node->state));
    cout << "Found a solution." << endl;
    cout << "Actual search time: " << timer
         << " [t=" << utils::g_timer << "]" << endl;
    Plan p;
    while (node->achiever) {
        p.push_back(node->achiever);
        node = node->predecessor;
    }
    std::reverse(p.begin(), p.end());
    SearchEngine::set_plan(p);
}

vector<pair<const GlobalOperator *, GlobalState>>
AStarSearch::get_successors(const GlobalState &s) {
    ++expanded_states;
    vector<OperatorID> applicable_ops;
    g_successor_generator->generate_applicable_ops(s, applicable_ops);

    vector<pair<const GlobalOperator *, GlobalState>> result;
    for (OperatorID op_id : applicable_ops) {
        const GlobalOperator &op = g_operators[op_id.get_index()];
        GlobalState succ_state = state_registry.get_successor_state(s, op);
        result.push_back(make_pair(&op, succ_state));
    }
    return result;
}

AStarSearchNode *AStarSearch::make_node(
    AStarSearchNode *n, const GlobalOperator *op, const GlobalState &s) {
    ++evaluated_states;
    int op_cost = get_adjusted_cost(*op);
    int h_value = heuristic->compute_heuristic(s);
    return new AStarSearchNode(s, n->g + op_cost, h_value, n, op);
}

int AStarSearch::h(const GlobalState &s) {
    int res = heuristic->compute_heuristic(s);
    if (res == Heuristic::DEAD_END) {
        res = INFTY;
        ++dead_ends;
    } else if (res < best_h) {
        cout << "New best heuristic value for "
             << heuristic->get_description() << ": " << res << endl;
        best_h = res;
    }
    return res;
}

void AStarSearch::search() {
    cout << "Conducting A* search" << endl;

    // insert your code here
    // open and distances are already created in the constructor
    if(best_h > h(get_initial_state())){
        open.insert(make_root_node());
    }

    while(!open.empty()){
        AStarSearchNode* n = open.pop_min();
        if(!distances.contains(n->state)){
            if(is_goal(n->state)){
                extract_solution(n);
                exit(0);
            }
            vector<pair<const GlobalOperator *, GlobalState>> sucessors = get_successors(n->state);
            for(pair<const GlobalOperator *, GlobalState> sucessor: sucessors){
                if(best_h > h(sucessor.second)){
                    AStarSearchNode* n_line = make_node(n, sucessor.first, sucessor.second);
                    open.insert(n_line);
                }
            }
        }
    }

    cout << "Completely explored search space -> no solution!" << endl;
    cout << "Actual search time: " << timer
         << " [t=" << utils::g_timer << "]" << endl;
}

static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "A* search",
        "The A* search engine that is implemented in the "
        "planning and optimization course");
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    Options heuristic_opts;
    heuristic_opts.set<shared_ptr<AbstractTask>>("transform", g_root_task());
    heuristic_opts.set("cache_estimates", true);
    Heuristic *h = new lm_cut_heuristic::LandmarkCutHeuristic(heuristic_opts);

    opts.set("h", h);
    return make_shared<AStarSearch>(opts);
}

static PluginShared<SearchEngine> _plugin("planopt_astar", _parse);

}
