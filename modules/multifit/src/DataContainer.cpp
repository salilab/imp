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
#include <IMP/helper/simplify_restraint.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/domino/RestraintGraph.h>

IMPMULTIFIT_BEGIN_NAMESPACE
DataContainer::DataContainer(const SettingsData &settings) {
  set_=settings;
  mdl_= new IMP::Model();
  IMP::atom::NonWaterPDBSelector sel;
  for(int i=0;i<settings.get_number_of_component_headers();i++) {
    ComponentHeader comp = settings.get_component_header(i);
    atom::Hierarchy mh = atom::read_pdb(
      comp.get_filename(),mdl_,sel);
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
  dens_ = helper::load_em_density_map(asmb_h.get_dens_fn().c_str(),
                                      asmb_h.get_spacing(),
                                      asmb_h.get_resolution());
  dens_ap_ = IMP::core::get_leaves(
               atom::read_pdb(asmb_h.get_pdb_coarse_ap_fn(),mdl_,
                              atom::CAlphaPDBSelector()));
  domino::read_junction_tree(asmb_h.get_junction_tree_fn(),&jt_);
}
IMPMULTIFIT_END_NAMESPACE
