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
#include <IMP/RestraintSet.h>
IMPDOMINO2_BEGIN_NAMESPACE
SubsetEvaluator::SubsetEvaluator(std::string name): Object(name){}
SubsetEvaluator::~SubsetEvaluator(){}
SubsetEvaluatorTable::~SubsetEvaluatorTable(){}





/*************************  MODEL ******************************/
namespace {
  class ModelSubsetEvaluator: public SubsetEvaluator {
    Pointer<const ModelSubsetEvaluatorTable> keepalive_;
    const internal::SubsetData &data_;
  public:
    ModelSubsetEvaluator(const ModelSubsetEvaluatorTable *t,
                         const internal::SubsetData &data):
      SubsetEvaluator("ModelSubsetEvaluator on "),
      keepalive_(t),
      data_(data) {
    }
    IMP_SUBSET_EVALUATOR(ModelSubsetEvaluator);
  };
  double ModelSubsetEvaluator::get_score(const SubsetState &state) const{
    return data_.get_score(state);
  }
  void ModelSubsetEvaluator::do_show(std::ostream &) const {
  }
}

ModelSubsetEvaluatorTable::ModelSubsetEvaluatorTable(Model *m,
                                                     ParticleStatesTable *pst):
  data_(m, m->get_root_restraint_set(),
        get_dependency_graph(RestraintsTemp(1, m->get_root_restraint_set())),
        pst) {
}

ModelSubsetEvaluatorTable::ModelSubsetEvaluatorTable(RestraintSet *rs,
                                                     ParticleStatesTable *pst):
  data_(rs->get_model(), rs,
        get_dependency_graph(RestraintsTemp(1, rs)),
        pst) {
}

SubsetEvaluator *
ModelSubsetEvaluatorTable::get_subset_evaluator(const Subset &s) const {
  return new ModelSubsetEvaluator(this, data_.get_subset_data(s));
}

void ModelSubsetEvaluatorTable::do_show(std::ostream &out) const{}



/*************************  ZERO ******************************/
namespace {

  class ZeroSubsetEvaluator: public SubsetEvaluator {
  public:
    ZeroSubsetEvaluator():
      SubsetEvaluator("ZeroSubsetEvaluator"){
    }
    IMP_SUBSET_EVALUATOR(ZeroSubsetEvaluator);
  };
  double ZeroSubsetEvaluator::get_score(const SubsetState &) const{
    return 0;
  }
  void ZeroSubsetEvaluator::do_show(std::ostream &) const {
  }
}

ZeroSubsetEvaluatorTable::ZeroSubsetEvaluatorTable() {
}


SubsetEvaluator *
ZeroSubsetEvaluatorTable::get_subset_evaluator(const Subset &s) const {
  return new ZeroSubsetEvaluator();
}

void ZeroSubsetEvaluatorTable::do_show(std::ostream &out) const{}

IMPDOMINO2_END_NAMESPACE
