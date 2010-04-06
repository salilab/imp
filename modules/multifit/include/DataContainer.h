/**
 *  \file DataContainer.h
 *  \brief stored multifit data
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_DATA_CONTAINER_H
#define IMPMULTIFIT_DATA_CONTAINER_H

#include "multifit_config.h"
#include "SettingsData.h"
#include "FittingSolutionRecord.h"
#include <IMP/base_types.h>
#include <IMP/em/DensityMap.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/domino/JunctionTree.h>
IMPMULTIFIT_BEGIN_NAMESPACE
//! Holds data structures neede for optimization
class IMPMULTIFITEXPORT DataContainer {
  public:
  DataContainer(){}
    DataContainer(const SettingsData &settings);
    atom::Hierarchy get_component(unsigned int i) const {
      IMP_USAGE_CHECK(i<mhs_.size(),"index out of range \n");
      return mhs_[i];
    }
    int get_number_of_components() const {
      return mhs_.size();}
    em::DensityMap* get_density_map() const {
      return dens_;}
    domino::JunctionTree get_junction_tree() const {
      return jt_;}
    FittingSolutionRecords get_fitting_solutions(Particle *p) {
      IMP_INTERNAL_CHECK(recs_.find(p) != recs_.end(),
       "no fitting records found for particle:"<<p->get_name()<<" \n");
      return recs_.find(p)->second;
    }
    Particle *get_density_anchor_point(int anchor_point_ind) {
      IMP_INTERNAL_CHECK(anchor_point_ind<(int)dens_ap_.size(),
                         "index out of range\n");
      return dens_ap_[anchor_point_ind];
    }
    inline int get_number_of_density_anchor_points() const {
      return dens_ap_.size();}
    Model *get_model() { return mdl_;}
  protected:
    Model *mdl_;
    atom::Hierarchies mhs_;
    std::map<Particle *,FittingSolutionRecords> recs_;
    em::DensityMap *dens_;
    IMP::Particles dens_ap_;
    domino::JunctionTree jt_;
};
IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_DATA_CONTAINER_H */
