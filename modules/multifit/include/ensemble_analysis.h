/**
 *  \file ensemble_analysis.h
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMULTIFIT_ENSEMBLE_ANALYSIS_H
#define IMPMULTIFIT_ENSEMBLE_ANALYSIS_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/multifit/SettingsData.h>
#include <IMP/multifit/FittingSolutionRecord.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/atom/distance.h>
#include <IMP/domino/DiscreteSampler.h>
#include <IMP/base_types.h>
#include <IMP/multifit/protein_anchors_mapping_reader.h>

IMPMULTIFIT_BEGIN_NAMESPACE

class IMPMULTIFITEXPORT Ensemble : public base::Object {
public:
  void load_combination(Ints fit_comb);
  void unload_combination(Ints fit_comb);
  void add_component_and_fits(
                              atom::Hierarchy mh,
                              const multifit::FittingSolutionRecords &fits);
  atom::Hierarchies get_molecules() const {return mhs_;}
  core::RigidBodies get_rigid_bodies() const {return rbs_;}
  float get_rmsd(const core::XYZs &second_xyz) const {
    return atom::get_rmsd(second_xyz,xyz_);}
  Ensemble(multifit::SettingsData *sd,
           const ProteinsAnchorsSamplingSpace &mapping_data);
  std::vector<Floats> score_by_restraints(Restraints rs,
                                          const IntsList &combinations);

  IMP_OBJECT_METHODS(Ensemble);

private:
  atom::Hierarchies mhs_;
  core::RigidBodies rbs_;
  core::XYZs xyz_;
  std::vector<multifit::FittingSolutionRecords> fits_;
  algebra::ReferenceFrame3Ds orig_rf_;
  ProteinsAnchorsSamplingSpace mapping_data_;
  multifit::SettingsData *sd_;
};
IMP_OBJECTS(Ensemble, Ensembles);

IMPMULTIFITEXPORT
Ensemble* load_ensemble(multifit::SettingsData *sd,Model *mdl,
              const ProteinsAnchorsSamplingSpace &mapping_data);

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_ENSEMBLE_ANALYSIS_H */
