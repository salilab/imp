/**
 *  \file proteomics_em_alignment_atomic.h
 *  \brief align proteomics graph to em density map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

//the alignment is from the em anchors to the proteomics graph
// Alignments get_alignments(const AnchorData &anchor_graph,
//                           const ProteomicsData &prot_graph) {
class IMPMULTIFITEXPORT ProteomicsEMAlignmentAtomic : public base::Object {
public:
  ProteomicsEMAlignmentAtomic(
                   const ProteinsAnchorsSamplingSpace &mapping_data,
                   multifit::SettingsData *asmb_data,
                   const AlignmentParams &align_param);
  void align();
  void add_all_restraints();
  void add_states_and_filters();
  void show_domino_merge_tree() const;
  domino::Assignments get_combinations(bool uniques=false) const;
  void set_density_map(em::DensityMap *dmap,float threshold) {
    dmap_=dmap;threshold_=threshold;
  }
  atom::Hierarchies get_molecules() const {return mhs_;}
  core::RigidBodies get_rigid_bodies() const {return rbs_;}
  /*  float load_configuration(int i) {
    cg_->load_configuration(cg_sorted_[i].first);
    return cg_sorted_[i].second;
    }*/
  //! load combination of states
  //!The order of the states should be the order
  void load_combination_of_states(const Ints &state4particles);
  void show_scores_header(std::ostream& ous=std::cout) const;
  /*  void show_scores(const domino::Assignment &a,
                   std::ostream& out=std::cout) const;
  */
  Model * get_model() {return mdl_;}
  //! If set to fast EV is calculated as penetration score and Fit
  //! restraint is decomposed
  void set_fast_scoring(bool state) {
    fast_scoring_=state;
  }

  IMP_OBJECT_INLINE(ProteomicsEMAlignmentAtomic, {IMP_UNUSED(out);}, {});

protected:
  RestraintsTemp get_alignment_restraints() const;
  Pointer<domino::RestraintCache> rc_;
  bool fast_scoring_;
  domino::ParticleStatesTable*
           set_particle_states_table(domino::SubsetFilterTables &filters);
  void load_atomic_molecules();
  //  void sort_configurations();
  ProteinsAnchorsSamplingSpace mapping_data_;
  OwnerPointer<multifit::ProteomicsData> prot_data_;
  Pointer<em::DensityMap> dmap_;
  double threshold_;
  atom::Hierarchies mhs_;
  core::RigidBodies rbs_;
  Pointer<Model> mdl_;
  AlignmentParams params_;
  IntsLists sampled_solutions_;//instead of cg
  domino::Assignments sampled_assignments_;//instead of sampled_solutions
  //configurations sorted by score
  std::vector<std::pair<int,float> >cg_sorted_;
  Pointer<RestraintSet> conn_rs_;
  Pointer<RestraintSet> conn_rs_with_filter_;
  Pointer<RestraintSet> xlink_rs_;
  Pointer<RestraintSet> xlink_rs_with_filter_;
  Pointer<RestraintSet> dummy_rs_;
  Pointer<RestraintSet> em_rs_;
  Pointer<RestraintSet> ev_rs_;
  RestraintsTemp jt_rs_;
  // Pointer<RestraintSet> ev_pruned_rs_;
  // Pointer<RestraintSet> rog_rs_;
  //Pointer<RestraintSet> other_rs_;//the other restraints
  //Pointer<RestraintSet> fit_rs_;//the other restraints
  OwnerPointer<domino::RestraintScoreSubsetFilterTable> all_rs_filt_;
  IntKey fit_state_key_,order_key_;
  bool restraints_set_,states_set_,filters_set_;
  OwnerPointer<domino::ParticleStatesTable> pst_;
  domino::SubsetFilterTables filters_;
  multifit::SettingsData *asmb_data_;
  IntPairs post_sampling_ev_pairs_;
  float ev_thr_;
};
IMP_OBJECTS(ProteomicsEMAlignmentAtomic, ProteomicsEMAlignmentAtomics);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_PROTEOMICS_EM_ALIGNMENT_ATOMIC_H */
