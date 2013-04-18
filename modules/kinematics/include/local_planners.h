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
#include <IMP/base/Object.h>

IMPKINEMATICS_BEGIN_NAMESPACE

/* general interface for planning the motion between two
   configuration nodes, each abstracted as a vector of DoFs
*/
class IMPKINEMATICSEXPORT LocalPlanner
: public IMP::base::Object
{
  IMP_OBJECT(LocalPlanner);

public:
  LocalPlanner(Model* model, DOFsSampler* dofs_sampler);

  // plan a path of valid intermediate nodes
  // from existing node q_from until the valid node that is
  // somewhay close to q_rand
  virtual
    std::vector<DOFValues> plan(DOFValues q_near,
                                DOFValues q_rand) = 0;

  bool is_valid(const DOFValues& values) {
    dofs_sampler_->apply(values);
    double score = sf_->evaluate_if_below(false, 0.0); //TODO: what here?
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
  between two nodes.
  @note the default path planning is over a linear interpolation between
        the nodes
*/
class IMPKINEMATICSEXPORT PathLocalPlanner : public LocalPlanner {
public:
  //! construct a path planner (default planning is linear)
  /**
     construct a path planner
     @param model the working model
     @param dofs_sampler an object for sampling node configurations
     @param directional_dof XX TODO XX
     @param save_step_interval the interval for recording nodes in the path.
                               For instance, 2 would mean save every second node
                               in the local plan.
   */
  PathLocalPlanner(Model* model, DOFsSampler* dofs_sampler,
                   DirectionalDOF* directional_dof,
                   int save_step_interval = 1);

protected:

    /** plan a linear path of intermediate nodes with a properly calculated
        step size from existing node q_from until the valid node that is
        found closest to q_rand (inclusive)
    */
  virtual std::vector<DOFValues> plan(DOFValues q_from, DOFValues q_rand);

private:
  DirectionalDOF* d_;
  unsigned int save_step_interval_; // if 0, save only last valid node
};

IMP_OBJECTS(LocalPlanner, LocalPlanners);
IMP_OBJECTS(PathLocalPlanner, PathLocalPlanners);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_LOCAL_PLANNERS_H */
