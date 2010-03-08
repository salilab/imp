/**
 *  \file SingleMultiFitRun.h
 *  \brief finding global minimum of a single mapping
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SINGLE_DOMINO_RUN_H
#define IMPMULTIFIT_SINGLE_DOMINO_RUN_H

#include <IMP/base_types.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/distance.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/domino/TransformationUtils.h>
#include <IMP/domino/TransformationMappedDiscreteSet.h>
#include <IMP/domino/TransformationCartesianProductSampler.h>
#include <IMP/domino/JunctionTree.h>
#include <IMP/domino/DominoOptimizer.h>
#include <IMP/em/DensityMap.h>
#include <IMP/ConfigurationSet.h>
#include <map>
#include "multifit_config.h"
#include "FittingSolutionRecord.h"

IMPMULTIFIT_BEGIN_NAMESPACE

typedef std::map<Particle*,Particle*> AnchorPointToComponentMapping;
typedef std::pair<Particle *,
                  std::vector<algebra::Transformation3D> >
   CompTransformationsPair;
typedef std::vector<CompTransformationsPair> CompTransformationsPairs;
typedef std::map<std::string,Particle *> NameToAnchorPointMapping;


//! Contains fitting solutions of all of the components of the system
class IMPMULTIFITEXPORT ComponentsFittingSolutionsContainer {
 public:
  ComponentsFittingSolutionsContainer(){}
  void add_solutions(atom::Hierarchy &comp,FittingSolutionRecords &comp_sols) {
    CompSolsMap::iterator finder_it = comp_sols_map_.find(comp.get_particle());
    if (finder_it ==comp_sols_map_.end()) {
       comp_sols_map_[comp.get_particle()]=comp_sols;
       mhs_.push_back(comp);
    }
    else {
      finder_it->second.insert(finder_it->second.end(),
                               comp_sols.begin(),
                               comp_sols.end());
    }
  }
  FittingSolutionRecords get_solutions(atom::Hierarchy &comp) const {
    IMP_INTERNAL_CHECK(
     comp_sols_map_.find(comp.get_particle()) != comp_sols_map_.end(),
     " no solutions found"<<std::endl);
    return comp_sols_map_.find(comp.get_particle())->second;
  }
protected:
  typedef std::map<IMP::Particle *,FittingSolutionRecords> CompSolsMap;
  atom::Hierarchies mhs_;
  CompSolsMap comp_sols_map_;
};

//! running domino optimization for a single mapping
class IMPMULTIFITEXPORT SingleDominoRun {
public:
  //! Constructor
  /**
   \param[in] anchor_comp_pairs pairs of
              an anchor point and its mapped components
   \param[in] components the components
   \param[in] jt the system junction tree (on the anchor points)
   \param[in] dmap the assembly density map
  */
  SingleDominoRun(
         const ParticlePairs &anchor_comp_pairs,
         atom::Hierarchies &components,
         domino::JunctionTree &jt,
         em::DensityMap *dmap);
  //! Optimize
  ConfigurationSet* optimize();
  //! Set tranformations for each component
  /**
   \param[in] comp_trans components and their transformations
 */
  void set_sampling_space(CompTransformationsPairs &comp_trans);
  //! Set Single and pairwise restraints according to
  //! the junction tree configuration
  void set_restraints();
  //! Get pairs of interacting anchor points according to the junction
  //! tree configuration
  ParticlePairs get_pairs_of_interacting_components() const;
protected:
  atom::Hierarchies components_;
  Particles components_ps_;
  AnchorPointToComponentMapping anchor_comp_map_;
  NameToAnchorPointMapping name_anchor_map_;
  domino::JunctionTree jt_;
  Model *mdl_;
  //managments stuff
  domino::TransformationUtils tu_;
  domino::TransformationMappedDiscreteSet *discrete_set_;
  domino::TransformationCartesianProductSampler sampler_;
  bool sampling_space_initialized_;
  //optimization
  domino::DominoOptimizer *d_opt_;
  //restraints
  bool restraints_initialized_;
  em::DensityMap *dmap_;
};

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_SINGLE_DOMINO_RUN_H */
