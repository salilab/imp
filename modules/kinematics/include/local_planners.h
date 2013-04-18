/**
 * \file local_planners \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_LOCAL_PLANNERS_H
#define IMPKINEMATICS_LOCAL_PLANNERS_H

#include "kinematics_config.h"
#include "DOFValues.h"
#include "directional_DOFs.h"
#include "DOFsSampler.h"

IMPKINEMATICS_BEGIN_NAMESPACE

/* general interface */
class IMPKINEMATICSEXPORT LocalPlanner{
public:
  LocalPlanner(Model* model, DOFsSampler* dofs_sampler) :
    model_(model), dofs_sampler_(dofs_sampler) {
    sf_= model_->create_model_scoring_function();
  }

  virtual std::vector<DOFValues> plan(DOFValues q_near,
                                      DOFValues q_rand) = 0;

  bool is_valid(const DOFValues& values) {
    dofs_sampler_->apply(values);
    double score = sf_->evaluate_if_below(false, 0.0) //TODO: what here?
    //std::cerr << "score = " << score << std::endl;
    if(score <= 0.000001)
      return true;
    return false;
  }

protected:
  OwnerPointer<Model> model_;
  DOFsSampler* dofs_sampler_;
  OwnerPointer<ScoringFunction> sf_;
};


/*
  Local planner implementation that samples conformations on a path
  between two nodes
*/
class IMPKINEMATICSEXPORT PathLocalPlanner : public LocalPlanner {
public:
  // default path sampling is linear
  PathLocalPlanner(Model* model, DOFsSampler* dofs_sampler,
                   DirectionalDOF* directional_dof,
                   int save_step_interval = 1) :
    LocalPlanner(model, dofs_sampler),
    d_(directional_dof),
    save_step_interval_(save_step_interval) {}

protected:

  // plan a path of intermediate nodes with a properly calculated
  // step size from existing node q_from until the valid node that is
  // found closest to q_rand (inclusive)
  virtual std::vector<DOFValues> plan(DOFValues q_from, DOFValues q_rand);

private:
  DirectionalDOF* d_;
  unsigned int save_step_interval_; // if 0, save only last valid node
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_LOCAL_PLANNERS_H */
