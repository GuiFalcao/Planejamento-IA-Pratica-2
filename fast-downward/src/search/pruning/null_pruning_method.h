#ifndef PRUNING_NULL_PRUNING_METHOD_H
#define PRUNING_NULL_PRUNING_METHOD_H

#include "../pruning_method.h"

class GlobalOperator;
class GlobalState;

namespace null_pruning_method {
class NullPruningMethod : public PruningMethod {
public:
    virtual void initialize(const std::shared_ptr<AbstractTask> &) override;
    virtual void prune_operators(const State &,
                                 std::vector<OperatorID> &) override {}
    virtual void prune_operators(const GlobalState &,
                                 std::vector<OperatorID> &) override {}
    virtual void print_statistics() const override {}
};
}

#endif
