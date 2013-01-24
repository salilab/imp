/**
 *  \file cn_rmsd.cpp  \brief High-level functions for RMSD calculation.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/cn_rmsd.h>
#include <IMP/cnmultifit/symmetry_utils.h>
#include <IMP/cnmultifit/internal/Parameters.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/force_fields.h>
#include <fstream>

IMPCNMULTIFIT_BEGIN_NAMESPACE

Floats get_rmsd_for_models(const std::string param_filename,
                           const std::string trans_filename,
                           const std::string ref_filename,
                           int start_model, int end_model)
{
  std::string protein_filename,surface_filename;
  int cn_symm_deg;

  std::cout<<"============= parameters ============"<<std::endl;
  std::cout<<"params filename : " << param_filename <<std::endl;
  std::cout<<"trans filename : " << trans_filename <<std::endl;
  std::cout<<"ref filename : " << ref_filename <<std::endl;
  std::cout<<"start index to rmsd : "<<start_model<<std::endl;
  std::cout<<"end index to rmsd : "<<end_model<<std::endl;
  std::cout<<"====================================="<<std::endl;

  internal::Parameters params(param_filename.c_str());
  protein_filename = params.get_unit_pdb_fn();
  cn_symm_deg = params.get_cn_symm();

  IMP_NEW(Model, mdl, ());
  atom::Hierarchy asmb_ref;
  atom::Hierarchies mhs;
  //read the monomer
  core::XYZs model_xyzs;
  for (int i=0;i<cn_symm_deg;i++) {
    atom::Hierarchy h = atom::read_pdb(protein_filename, mdl,
                                       new atom::CAlphaPDBSelector());
    atom::Chain c= atom::get_chain(atom::Residue(atom::get_residue(
                                     atom::Atom(core::get_leaves(h)[0]))));
    c.set_id(char(65+i));
    atom::add_radii(h);
    atom::setup_as_rigid_body(h);
    mhs.push_back(h);
    Particles leaves = core::get_leaves(h);
    for(Particles::iterator it = leaves.begin(); it != leaves.end(); it++) {
      model_xyzs.push_back(core::XYZ(*it));
    }
  }
  //read the transformations
  multifit::FittingSolutionRecords recs
               = multifit::read_fitting_solutions(trans_filename.c_str());
  //read the reference structure
  asmb_ref=atom::read_pdb(ref_filename,mdl,new atom::CAlphaPDBSelector());
  atom::Hierarchies ref_chains = atom::Hierarchies(atom::get_by_type(
                                      asmb_ref, atom::CHAIN_TYPE));
  std::cout<<"number of records:"<<recs.size()<<std::endl;
  Floats rmsd;
  std::ofstream out;
  out.open("rmsd.output");
  if (end_model < 0 || end_model >= static_cast<int>(recs.size())) {
    end_model = recs.size() - 1;
  }

  for (int i = start_model; i >= 0 && i <= end_model; ++i) {
    algebra::Transformation3D t1 = recs[i].get_dock_transformation();
    algebra::Transformation3D t2 = recs[i].get_fit_transformation();
    algebra::Transformation3D t2_inv = t1.get_inverse();
    transform_cn_assembly(mhs,t1);
    for(unsigned int j=0;j<model_xyzs.size();j++){
      model_xyzs[j].set_coordinates(t2.get_transformed(
                                model_xyzs[j].get_coordinates()));
    }
    std::cout<<mhs.size()<<","<<ref_chains.size()<<std::endl;
    Float cn_rmsd=get_cn_rmsd(mhs,ref_chains);
    out<<" trans:"<<i<<" rmsd: "<<cn_rmsd<<" cc: "
       << 1.-recs[i].get_fitting_score()<<std::endl;
    rmsd.push_back(cn_rmsd);
    for(unsigned int j=0;j<model_xyzs.size();j++){
      model_xyzs[j].set_coordinates(t2_inv.get_transformed(
                                model_xyzs[j].get_coordinates()));
    }
    /*      std::stringstream ss;
    ss<<"asmb_"<<i<<".pdb";
    atom::write_pdb(mhs,ss.str());*/
    transform_cn_assembly(mhs,t1.get_inverse());
  }

  out.close();
  return rmsd;
}

IMPCNMULTIFIT_END_NAMESPACE
