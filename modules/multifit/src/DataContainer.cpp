/**
 *  \file DataContainer.cpp
 *  \brief stored multifit data
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/DataContainer.h>
#include <IMP/atom/force_fields.h>
#include <IMP/atom/pdb.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/domino/RestraintGraph.h>

IMPMULTIFIT_BEGIN_NAMESPACE
DataContainer::DataContainer(Model *mdl,
                             const SettingsData &settings) {
  set_=settings;
  mdl_= mdl;
  //IMP::atom::NonWaterNonHydrogenPDBSelector sel;
  IMP::atom::CAlphaPDBSelector sel;
  for(int i=0;i<settings.get_number_of_component_headers();i++) {
    ComponentHeader comp = settings.get_component_header(i);
    IMP_LOG(VERBOSE,"adding component:"<<comp.get_filename()<<std::endl);
    atom::Hierarchy mh;
    try{
      mh = atom::read_pdb(
      comp.get_filename(),mdl_,sel);
    }
    catch(...){
      IMP_WARN("some atoms were not read for file:"<<comp.get_filename()<<"\n");
    }
    atom::add_radii(mh);
    mh.get_particle()->add_attribute(domino::node_name_key(),
           settings.get_component_header(i).get_name());
    atom::setup_as_rigid_body(mh);
    mhs_.push_back(mh);
    atom::Hierarchy mh_ref;
    if (comp.get_reference_fn() != "") {
      mh_ref = atom::read_pdb(comp.get_reference_fn(),mdl_,sel);
      atom::add_radii(mh_ref);
    }
    mhs_ref_.push_back(mh_ref);
    recs_[mh.get_particle()]=
       read_fitting_solutions(comp.get_transformations_fn().c_str());
  }
  AssemblyHeader asmb_h = settings.get_assembly_header();
  dens_ = em::read_map(asmb_h.get_dens_fn().c_str());
  dens_->get_header_writable()->set_resolution(asmb_h.get_resolution());
  dens_->update_voxel_size(asmb_h.get_spacing());

  dens_ap_ = IMP::core::get_leaves(
               atom::read_pdb(asmb_h.get_pdb_coarse_ap_fn(),mdl_,
                              atom::CAlphaPDBSelector()));
  domino::read_junction_tree(asmb_h.get_junction_tree_fn(),&jt_);
}
IMPMULTIFIT_END_NAMESPACE
