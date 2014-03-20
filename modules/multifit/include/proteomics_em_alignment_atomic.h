/**
 *  \file IMP/multifit/proteomics_em_alignment_atomic.h
 *  \brief align proteomics graph to em density map
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_PROTEOMICS_EM_ALIGNMENT_ATOMIC_H
#define IMPMULTIFIT_PROTEOMICS_EM_ALIGNMENT_ATOMIC_H

#include <IMP/multifit/ComplementarityRestraint.h>
#include <IMP/multifit/proteomics_reader.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include "AlignmentParams.h"
#include <IMP/core/LeavesRefiner.h>
#include <IMP/multifit/anchors_reader.h>
#include <IMP/multifit/SettingsData.h>
#include "protein_anchors_mapping_reader.h"
#include <IMP/domino/DominoSampler.h>
#include <IMP/em/DensityMap.h>
#include <IMP/multifit/multifit_config.h>
#include <algorithm>
#include <boost/scoped_ptr.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Align proteomics graph to EM density map
/** The alignment is from the em anchors to the proteomics graph
 */
class IMPMULTIFITEXPORT ProteomicsEMAlignmentAtomic : public base::Object {
 public:
  ProteomicsEMAlignmentAtomic(const ProteinsAnchorsSamplingSpace &mapping_data,
                              multifit::SettingsData *asmb_data,
                              const AlignmentParams &align_param);
  void align();
  void add_all_restraints();
  void add_states_and_filters();
  void show_domino_merge_tree() const;
  domino::Assignments get_combinations(bool uniques = false) const;
  void set_density_map(em::DensityMap *dmap, float threshold) {
    dmap_ = dmap;
    threshold_ = threshold;
  }
  atom::Hierarchies get_molecules() const { return mhs_; }
  core::RigidBodies get_rigid_bodies() const { return rbs_; }
  /*  float load_configuration(int i) {
    cg_->load_configuration(cg_sorted_[i].first);
    return cg_sorted_[i].second;
    }*/
  //! load combination of states
  //!The order of the states should be the order
  void load_combination_of_states(const Ints &state4particles);
  void show_scores_header(std::ostream &ous = std::cout) const;
  /*  void show_scores(const domino::Assignment &a,
                   std::ostream& out=std::cout) const;
  */
  kernel::Model *get_model() { return mdl_; }
  //! If set to fast EV is calculated as penetration score and Fit
  //! restraint is decomposed
  void set_fast_scoring(bool state) { fast_scoring_ = state; }

  IMP_OBJECT_METHODS(ProteomicsEMAlignmentAtomic);

 protected:
  kernel::RestraintsTemp get_alignment_restraints() const;
  base::Pointer<domino::RestraintCache> rc_;
  bool fast_scoring_;
  domino::ParticleStatesTable *set_particle_states_table(
      domino::SubsetFilterTables &filters);
  void load_atomic_molecules();
  //  void sort_configurations();
  ProteinsAnchorsSamplingSpace mapping_data_;
  base::PointerMember<multifit::ProteomicsData> prot_data_;
  base::Pointer<em::DensityMap> dmap_;
  double threshold_;
  atom::Hierarchies mhs_;
  core::RigidBodies rbs_;
  base::Pointer<Model> mdl_;
  AlignmentParams params_;
  IntsLists sampled_solutions_;              // instead of cg
  domino::Assignments sampled_assignments_;  // instead of sampled_solutions
  // configurations sorted by score
  std::vector<std::pair<int, float> > cg_sorted_;
  base::Pointer<kernel::RestraintSet> conn_rs_;
  base::Pointer<kernel::RestraintSet> conn_rs_with_filter_;
  base::Pointer<kernel::RestraintSet> xlink_rs_;
  base::Pointer<kernel::RestraintSet> xlink_rs_with_filter_;
  base::Pointer<kernel::RestraintSet> dummy_rs_;
  base::Pointer<kernel::RestraintSet> em_rs_;
  base::Pointer<kernel::RestraintSet> ev_rs_;
  kernel::RestraintsTemp jt_rs_;
  // base::Pointer<kernel::RestraintSet> ev_pruned_rs_;
  // base::Pointer<kernel::RestraintSet> rog_rs_;
  // Pointer<kernel::RestraintSet> other_rs_;//the other restraints
  // Pointer<kernel::RestraintSet> fit_rs_;//the other restraints
  base::PointerMember<domino::RestraintScoreSubsetFilterTable> all_rs_filt_;
  IntKey fit_state_key_, order_key_;
  bool restraints_set_, states_set_, filters_set_;
  base::PointerMember<domino::ParticleStatesTable> pst_;
  domino::SubsetFilterTables filters_;
  multifit::SettingsData *asmb_data_;
  IntPairs post_sampling_ev_pairs_;
  float ev_thr_;
};
IMP_OBJECTS(ProteomicsEMAlignmentAtomic, ProteomicsEMAlignmentAtomics);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_PROTEOMICS_EM_ALIGNMENT_ATOMIC_H */
