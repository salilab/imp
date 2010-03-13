/**
 *  \file SingleDominoRun.cpp
 *  \brief finding global minimum of a single mapping
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/SingleDominoRun.h>
#include <IMP/multifit/weighted_excluded_volume.h>
#include <IMP/helper/simplify_restraint.h>

IMPMULTIFIT_BEGIN_NAMESPACE

ParticlePairs SingleDominoRun::get_pairs_of_interacting_components() const
{
  ParticlePairs pairs;
  for(int comp_ind=0;comp_ind<jt_.get_number_of_nodes();comp_ind++) {
    for( int j1=0;
          j1<jt_.get_number_of_components(comp_ind);j1++) {
      std::string name1=jt_.get_component_name(comp_ind,j1);
      Particle *comp1 =
        anchor_comp_map_.find(name_anchor_map_.find(name1)->second)->second;
      for( int j2=j1+1;
            j2<jt_.get_number_of_components(comp_ind);j2++) {
        std::string name2=jt_.get_component_name(comp_ind,j2);
        Particle *comp2 =
          anchor_comp_map_.find(name_anchor_map_.find(name2)->second)->second;
        pairs.push_back(ParticlePair(comp1,comp2));
      }
    }
  }
  return pairs;
}

void SingleDominoRun::set_restraints() {
  IMP_INTERNAL_CHECK(restraints_initialized_==false,
              "The restraints have already been initalized"<<std::endl);
  Restraints rsrs;
  //set pair restraints
  ParticlePairs pairs = get_pairs_of_interacting_components();
  Float pairwise_weight=0.2;
  Float single_weight=1.;
  for(ParticlePairs::iterator it = pairs.begin(); it != pairs.end(); it++)
    {
      IMP_LOG(TERSE,"setting pairwise restraint between:"<<
                    it->get(0)->get_name()<< " and " <<
                    it->get(1)->get_name() << std::endl);
      Particles ps;
      ps.push_back(it->get(0));
      ps.push_back(it->get(1));
      d_opt_->add_restraint(
        multifit::create_weighted_excluded_volume_restraint(
            core::RigidBody(ps[0]),
            core::RigidBody(ps[1])),
        ps,
        pairwise_weight);
  }
  //set single restraints
  for(atom::Hierarchies::iterator it=components_.begin();
      it != components_.end(); it++) {
    IMP_LOG(TERSE,"setting EM restraint for:"<<
                  it->get_particle()->get_name()<<std::endl);
    helper::SimpleEMFit simple_fit=
       helper::create_simple_em_fit(*it,dmap_);
    mdl_->add_restraint(simple_fit.get_restraint());
    d_opt_->add_restraint(simple_fit.get_restraint(),
                          it->get_particle(),single_weight);
  }
  restraints_initialized_=true;
}


ConfigurationSet* SingleDominoRun::optimize() {
  int num_solutions=5;
  IMP_NEW(ConfigurationSet,conf_set,(mdl_,"conf_set.txt"));
  set_restraints();
  d_opt_->set_sampling_space(&sampler_);
  d_opt_->set_number_of_solutions(num_solutions);
  d_opt_->optimize(num_solutions);
  for(int i=0;i<num_solutions;i++) {
    d_opt_->move_to_opt_comb(i);
    conf_set->save_configuration();
  }
  return conf_set;
}

SingleDominoRun::SingleDominoRun(
   const ParticlePairs &anchor_comp_pairs,
   atom::Hierarchies &components,
   domino::JunctionTree &jt,
   em::DensityMap *dmap) {
  IMP_INTERNAL_CHECK(components.size()>0,"no components provided"<<std::endl);
  jt_=jt;
  components_ = components;
  for(atom::Hierarchies::iterator it = components_.begin();
     it != components_.end();it++) {
    components_ps_.push_back(it->get_particle());
  }
  mdl_=components_[0]->get_model();
  dmap_=dmap;
  for(ParticlePairs::const_iterator it = anchor_comp_pairs.begin();
      it != anchor_comp_pairs.end();it++) {
    anchor_comp_map_[it->get(0)]=it->get(1);
  }
  tu_=domino::TransformationUtils(components_,true);
  discrete_set_ = new domino::TransformationMappedDiscreteSet(components_);
  sampling_space_initialized_=false;
  restraints_initialized_=false;
  //set the name to anchor mapping
  StringKey name_key("name"); //TODO - have it in a better place, decorator ?
  for(AnchorPointToComponentMapping::iterator it = anchor_comp_map_.begin();
      it != anchor_comp_map_.end();it++) {
    Particle *anchor_point = it->first;
    anchor_point->show();
    IMP_INTERNAL_CHECK(anchor_point->has_attribute(name_key),
       "no name has been assigned to the anchor point");
    std::string anchor_point_name = anchor_point->get_value(name_key);
    IMP_INTERNAL_CHECK(name_anchor_map_.find(anchor_point_name) ==
       name_anchor_map_.end(),"the name has already been mapped");
    name_anchor_map_[anchor_point_name]=anchor_point;
  }
 //is it ok to pass null here ?
  d_opt_ = new domino::DominoOptimizer(jt_,mdl_,NULL);
}
void SingleDominoRun::set_sampling_space(
  CompTransformationsPairs &comp_trans) {
  IMP_INTERNAL_CHECK(comp_trans.size()>0,
            "No data to generate sampling space from" <<std::endl);
  IMP_INTERNAL_CHECK(sampling_space_initialized_==false,
            "the sampling space has already been set"<<std::endl);
  Model *mdl = comp_trans[0].first->get_model();
  IntKey orig_ind_key = IntKey("orig_ind");
  //add valid transformation for each rigid body
  for(CompTransformationsPairs::iterator it = comp_trans.begin();
      it != comp_trans.end(); it++) {
    std::vector<algebra::Transformation3D> ts = it->second;
    Particle *comp=it->first;
    for(unsigned int i=0;i<ts.size();i++) {
      Particle *state_p = new Particle(mdl);
      domino::Transformation::setup_particle(state_p,ts[i],i);
      state_p->add_attribute(orig_ind_key,i);//TODO - why is it needed?
      discrete_set_->add_mapped_state(comp,state_p);
    }
  }
  sampler_ = domino::TransformationCartesianProductSampler(
    discrete_set_,components_,true);
  sampling_space_initialized_=true;
}

IMPMULTIFIT_END_NAMESPACE
