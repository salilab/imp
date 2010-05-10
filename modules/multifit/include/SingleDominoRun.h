/**
 *  \file SingleDominoRun.h
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
#include "DataContainer.h"
#include <IMP/domino/RestraintEvaluatorFromFile.h>

IMPMULTIFIT_BEGIN_NAMESPACE
typedef std::map<Particle *,Particle *> ParticleParticleMapping;
typedef std::pair<Particle *,FittingSolutionRecords> CompFitRecordsPair;
typedef std::vector<CompFitRecordsPair> CompFitRecordsPairs;
typedef std::map<std::string,Particle *> NameToAnchorPointMapping;
typedef std::map<Particle *,std::string> AnchorPointToNameMapping;


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
   \param[in] dc a container holding all data structures
              needed for a MultiFit run
  */
  SingleDominoRun(
         const DataContainer &dc);
  //! Optimize
  domino::DominoOptimizer* optimize(int num_solutions);

  //! Get pairs of interacting anchor points according to the junction
  //! tree configuration
  ParticlePairs get_pairs_of_interacting_components(
    const ParticlePairs &anchor_comp_pairs) const;
  //! Run optimization
  /**
  \param[in] anchor_comp_pairs mapping from components to anchor points
  \param[in] distance only consider fitting solutions that are at most
                      distance from the mapped anchor point
   \param[in] num_solutions the number of models to keep
   \param[in] init_restraint_values_from_files if True the restraints values
               have been precalculated and kept in files
   */
  domino::DominoOptimizer* run(ParticlePairs anchor_comp_pairs,
     Float distance,
     int num_solutions,
     bool init_restraint_values_from_files = false);
  //! Set the path of the restraint score directory
  void set_restraint_score_directory_path(const std::string &fn) {
    path_=fn;
  }
  void set_ev_restraint_weight(Float w) {
    ev_weight_ = w;
  }
  void set_em_restraint_weight(Float w) {
    em_weight_ = w;
  }
  std::vector<algebra::Transformation3D>
   get_solution_as_transformations(int sol) {
     return sampler_.get_transformations_of_state(
             d_opt_->get_optimum_configuration(sol));
  }
protected:
  void clear_optimization_data();
  void setup();
  //! Set tranformations for each component
  /**
   \param[in] distance only consider transformations that are close
                       to the anchor point
 */
  void set_sampling_space(const ParticlePairs &anchor_comp_pairs,
                          Float distance);
  //! Set Single and pairwise restraints according to
  //! the junction tree configuration
  void set_restraints(const ParticlePairs &anchor_comp_pairs,
                      domino::RestraintEvaluatorFromFile *re=NULL);
  atom::Hierarchies components_;
  //Particles components_ps_;
  //  ParticleParticleMapping anchor_comp_map_;
  //ParticleParticleMapping comp_anchor_map_;
  AnchorPointToNameMapping anchor_name_map_;
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
  bool is_setup_;
  em::DensityMap *dmap_;
  DataContainer dc_;
  std::string path_;
  Float ev_weight_ , em_weight_;
};

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_SINGLE_DOMINO_RUN_H */
