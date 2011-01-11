/**
 *  \file DataContainer.h
 *  \brief stored multifit data
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
#include <IMP/domino1/JunctionTree.h>
IMPMULTIFIT_BEGIN_NAMESPACE
//! Holds data structures neede for optimization
class IMPMULTIFITEXPORT DataContainer : public Object{
  public:
  DataContainer(){}
  DataContainer(Model *mdl,const SettingsData &settings);
    atom::Hierarchy get_component(unsigned int i) const {
      IMP_USAGE_CHECK(i<mhs_.size(),"index "<<i<<" is out of range "
                      << "[0,"<<mhs_.size()<<"]\n");
      return mhs_[i];
    }
    atom::Hierarchy get_component_reference(unsigned int i) const {
      IMP_USAGE_CHECK(i<mhs_ref_.size(),"index of ref "<< i
                      <<" is out of range [0,"<<mhs_.size()<<"]\n");
      return mhs_ref_[i];
    }
    int get_number_of_components() const {
      return mhs_.size();}
    em::DensityMap* get_density_map() const {
      return dens_;}
    domino1::JunctionTree get_junction_tree() const {
      return jt_;}
    FittingSolutionRecords get_fitting_solutions(Particle *p) const {
      IMP_INTERNAL_CHECK(recs_.find(p) != recs_.end(),
       "no fitting records found for particle:"<<p->get_name()<<" \n");
      return recs_.find(p)->second;
    }
    Particle *get_density_anchor_point(int anchor_point_ind) const {
      IMP_INTERNAL_CHECK(anchor_point_ind<(int)dens_ap_.size(),
                         "index out of range\n");
      return dens_ap_[anchor_point_ind];
    }
    inline int get_number_of_density_anchor_points() const {
      return dens_ap_.size();}
    Model *get_model() { return mdl_;}
    void show(std::ostream& out=std::cout) const {
      out<<"DataContainer"<<std::endl;
    }
    SettingsData get_settings() const { return set_;}
    IMP_OBJECT_INLINE(DataContainer, show(out),{});
  protected:
    IMP::internal::OwnerPointer<Model> mdl_;
    atom::Hierarchies mhs_;
    atom::Hierarchies mhs_ref_;
    std::map<Particle *,FittingSolutionRecords> recs_;
    IMP::internal::OwnerPointer<em::DensityMap> dens_;
    IMP::Particles dens_ap_;
    domino1::JunctionTree jt_;
    SettingsData set_;
};
IMP_OBJECTS(DataContainer,DataContainers);
IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_DATA_CONTAINER_H */
