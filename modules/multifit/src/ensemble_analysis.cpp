/**
 *  \file ensemble_analysis.cpp
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/multifit/ensemble_analysis.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/Selection.h>
#include <IMP/multifit/RadiusOfGyrationRestraint.h>
#include <boost/progress.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

#if 0
namespace {
atom::Hierarchy create_protein(
                               const std::string &name,int prot_ind,
                               int num_beads,float bead_radius,Model *mdl,
                               IntKey order_key, IntKey path_ind_key,
                               IntKey anchor_key,
                               float max_conn_rest_val){

  IMP_LOG_TERSE("create protein "<<name<<" with "<<num_beads<<" beads"
                <<std::endl);
  std::cout<<"create protein "<<name<<" with "<<num_beads<<" beads"<<std::endl;
  atom::Hierarchy prot=
    atom::Hierarchy::setup_particle(new Particle(mdl));
  prot->set_name(name);
  prot->add_attribute(order_key,prot_ind);
  prot->add_attribute(path_ind_key,-1);
  atom::Selections sel;
  //create the beads
  std::cout<<"number of children "<<num_beads<<std::endl;
  for (int i=0;i<num_beads;i++){
    core::XYZR bead_child=core::XYZR::setup_particle(
             new Particle(mdl),
             algebra::Sphere3D(algebra::Vector3D(0,0,0),
                               bead_radius));
    atom::Mass::setup_particle(bead_child,3);
    bead_child->add_attribute(anchor_key,0);
    core::XYZ(bead_child).set_coordinates_are_optimized(true);
    atom::Hierarchy::setup_particle(bead_child);
    std::stringstream ss;
    ss<<name<<"."<<i;
    bead_child->set_name(ss.str());
    prot.add_child(atom::Hierarchy(bead_child));
    sel.push_back(atom::Selection(atom::Hierarchy(bead_child)));
  }
  int k=1;//todo - make this a parameter!!
  // //TODO - do we need this restraint
   if (sel.size()>1){

     Restraint *r = atom::create_connectivity_restraint(sel,k);
     IMP_INTERNAL_CHECK(r!=NULL,
                        "Create connectivity failed for protein:"<<name
                        <<" failed \n");
    mdl->add_restraint(r);
     r->set_log_level(VERBOSE);

     std::stringstream ss;
     ss<<"conn."<<name;
     r->set_name(ss.str());
     //only allow the particles to penetrate or separate by 1 angstrom
      IMP_LOG_VERBOSE(
              "max conn restraint:"<<max_conn_rest_val<<std::endl);
      mdl->set_maximum_score(r,max_conn_rest_val);
   }
  return prot;
}

}
#endif

Ensemble* load_ensemble(multifit::SettingsData *sd,Model *mdl,
                               const ProteinsAnchorsSamplingSpace &mapping_data)
{
  IMP_NEW(Ensemble, ens, (sd,mapping_data));
  for (int i=0;i<(int)sd->get_number_of_component_headers();i++) {
    atom::Hierarchy mh = atom::read_pdb(sd->get_component_header(i)
                                            ->get_filename(),mdl);
    mh->set_name(sd->get_component_header(i)->get_name());
    mh->add_attribute(StringKey("filename"),
                      sd->get_component_header(i)->get_filename());
    //create_rigid_body
    atom::create_rigid_body(mh);
    multifit::FittingSolutionRecords fits =
      multifit::read_fitting_solutions(sd->get_component_header(i)
                                           ->get_transformations_fn().c_str());
    ens->add_component_and_fits(mh,fits);
  }
  return ens.release();
}

void Ensemble::load_combination(Ints fit_comb) {
  //  std::cout<<"For combination:"<<fit_comb<<std::endl;
  //  std::cout<<"load_combination "<<fit_comb.size()<<" "<< mhs_.size()
  //           <<std::endl;
  IMP_USAGE_CHECK(fit_comb.size()==mhs_.size(),
    "Combination size does not match "<<
    fit_comb.size()<<" " <<mhs_.size()<<"\n");
  for(int i=0;i<(int)mhs_.size();i++) {
    //    std::cout<<"i:"<<fit_comb[i]<<" "<< fits_[i].size()<<std::endl;
    IMP_USAGE_CHECK(fit_comb[i]<(int)fits_[i].size(),
                    "Fit for mol "<<i<<" is out of range\n");
    core::RigidBody rb = rbs_[i];
    // std::cout<<sd_->get_component_header(i)->get_name()<<std::endl;
    // std::cout<<mhs_[i]->get_name()<<std::endl;
    IntsList fit_inds =
      mapping_data_.get_paths_for_protein(
                             sd_->get_component_header(i)->get_name());
    //    std::cout<<fit_inds.size()<<" | " << std::endl;
    int fit_ind=fit_inds[fit_comb[i]][0];
    // std::cout<<"fit ind:"<<fit_ind<<std::endl;
    // std::cout<<"i:"<<i<<" trans: "<< fit_ind<<" "
    //          << fits_[i][fit_ind].get_fit_transformation()<<std::endl;
    core::transform(rb,fits_[i][fit_ind].get_fit_transformation());
  }
}


void Ensemble::unload_combination(Ints fit_comb) {
  IMP_USAGE_CHECK(fit_comb.size()==mhs_.size(),
                  "Combination size does not match\n");
  for(int i=0;i<(int)mhs_.size();i++) {
    IMP_USAGE_CHECK(fit_comb[i]<(int)fits_[i].size(),
                    "Fit for mol "<<i<<" is out of range\n");
    core::RigidBody rb = rbs_[i];
    IntsList fit_inds =
      mapping_data_.get_paths_for_protein(
                            sd_->get_component_header(i)->get_name());
    int fit_ind=fit_inds[fit_comb[i]][0];
    core::transform(rb,
                    fits_[i][fit_ind].get_fit_transformation().get_inverse());
  }
}


void Ensemble::add_component_and_fits(
       atom::Hierarchy mh, const multifit::FittingSolutionRecords &fits) {
  mhs_.push_back(mh);
  std::cout<<"Adding molecule:"<<mh->get_name()<<std::endl;
  core::XYZs mh_xyz = core::XYZs(core::get_leaves(mh));
  xyz_.insert(xyz_.end(),mh_xyz.begin(),mh_xyz.end());
  core::RigidBody rb = core::RigidMember(mh_xyz[0]).get_rigid_body();
  fits_.push_back(fits);
  orig_rf_.push_back(rb.get_reference_frame());
  rbs_.push_back(rb);
}

Ensemble::Ensemble(multifit::SettingsData *sd,
     const ProteinsAnchorsSamplingSpace &mapping_data)
     : base::Object("Ensemble%1%"), mapping_data_(mapping_data),sd_(sd)
{
}

std::vector<Floats> Ensemble::score_by_restraints(
                        Restraints rs, const IntsList &combinations ) {
  std::vector<Floats> scores(combinations.size());
  boost::progress_display show_progress(combinations.size());
  for(int i=0;i<(int)combinations.size();i++) {
    load_combination(combinations[i]);
    std::cout<<"i:"<<i<<"  comb:"<<combinations[i]<<std::endl;
    ++show_progress;
    std::cout<<"===step1"<<std::endl;
    Floats comb_scores(rs.size());
    std::cout<<"===step2"<<std::endl;
    for(int j=0;j<(int)rs.size();j++) {
      std::cout<<"j is:"<<rs[j]->get_name()<<std::endl;
      comb_scores[j]=rs[j]->evaluate(false);
    }
    std::cout<<"===step3"<<std::endl;
    scores[i]=comb_scores;
    std::cout<<"===step4"<<std::endl;
    unload_combination(combinations[i]);
    std::cout<<"===step5"<<std::endl;
  }
  return scores;
}

IMPMULTIFIT_END_NAMESPACE
