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
#include <IMP/base_types.h>
#include <IMP/em/DensityMap.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/domino/JunctionTree.h>
IMPMULTIFIT_BEGIN_NAMESPACE
//! Holds data structures neede for optimization
class IMPMULTIFITEXPORT DataContainer {
  public:
    DataContainer(const SettingsData &settings);
    atom::Hierarchy get_component(int i) const {
      IMP_USAGE_CHECK(i<mhs_.size(),"index out of range \n");
      return mhs_[i];
    }
    int get_number_of_components() const {
      return mhs_.size();}
    em::DensityMap* get_density_map() const {
      return dens_;}
    domino::JunctionTree get_junction_tree() const {
      return jt_;}
  protected:
    Model *mdl_;
    atom::Hierarchies mhs_;
    em::DensityMap *dens_;
    IMP::Particles dens_ap_;
    domino::JunctionTree jt_;
};
IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_DATA_CONTAINER_H */
