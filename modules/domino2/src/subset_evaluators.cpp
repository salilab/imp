/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/subset_evaluators.h>
#include <IMP/domino2/utility.h>
#include <IMP/domino2/DominoSampler.h>
#include <IMP/Restraint.h>

IMPDOMINO2_BEGIN_NAMESPACE
SubsetEvaluator::SubsetEvaluator(std::string name): Object(name){}
SubsetEvaluator::~SubsetEvaluator(){}
SubsetEvaluatorTable::~SubsetEvaluatorTable(){}






namespace {
  class ModelSubsetEvaluator: public SubsetEvaluator {
    Pointer<const ModelSubsetEvaluatorTable> keepalive_;
    const internal::SubsetData &data_;
    double max_;
  public:
    ModelSubsetEvaluator(const ModelSubsetEvaluatorTable *t,
                         const internal::SubsetData &data,
                         double max):
      SubsetEvaluator("ModelSubsetEvaluator on "),
      keepalive_(t),
      data_(data), max_(max) {
    }
    IMP_SUBSET_EVALUATOR(ModelSubsetEvaluator);
  };
  double ModelSubsetEvaluator::get_score(const SubsetState &state) const{
    return data_.get_score(state, max_);
  }
  void ModelSubsetEvaluator::do_show(std::ostream &) const {
  }
}

ModelSubsetEvaluatorTable::ModelSubsetEvaluatorTable(Model *m,
                                                     ParticleStatesTable *pst):
  data_(m, m->get_dependency_graph(), pst) {
}

SubsetEvaluator *
ModelSubsetEvaluatorTable::get_subset_evaluator(const Subset &s) const {
  return new ModelSubsetEvaluator(this, data_.get_subset_data(s),
                                         get_sampler()->get_maximum_score());
}

void ModelSubsetEvaluatorTable::do_show(std::ostream &out) const{}

IMPDOMINO2_END_NAMESPACE
