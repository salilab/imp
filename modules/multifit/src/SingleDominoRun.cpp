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
#include <IMP/multifit/fitting_tools.h>
#include <IMP/domino/RestraintEvaluator.h>
#include <IMP/domino/Transformation.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/multifit/restraints_utils.h>

IMPMULTIFIT_BEGIN_NAMESPACE

ParticlePairs SingleDominoRun::get_pairs_of_interacting_components(
const ParticlePairs &anchor_comp_pairs) const
{
  ParticleParticleMapping anchor_comp_map;
  NameToAnchorPointMapping name_anchor_map;
  //setup the name-anchor map
  for(ParticlePairs::const_iterator it = anchor_comp_pairs.begin();
      it != anchor_comp_pairs.end();it++) {
    Particle *anchor_point=it->get(0);
    Particle *comp = it->get(1);
    std::string anchor_point_name = anchor_name_map_.find(anchor_point)->second;
    anchor_comp_map[anchor_point]=comp;
  }
  for(AnchorPointToNameMapping::const_iterator it = anchor_name_map_.begin();
         it != anchor_name_map_.end(); it++) {
    name_anchor_map[it->second]=it->first;
  }
  ParticlePairs pairs;
  for(int comp_ind=0;comp_ind<jt_.get_number_of_nodes();comp_ind++) {
    for( int j1=0;
          j1<jt_.get_number_of_components(comp_ind);j1++) {
      std::string name1=jt_.get_component_name(comp_ind,j1);
      Particle *comp1 =
        anchor_comp_map.find(name_anchor_map.find(name1)->second)->second;
      for( int j2=j1+1;
            j2<jt_.get_number_of_components(comp_ind);j2++) {
        std::string name2=jt_.get_component_name(comp_ind,j2);
        Particle *comp2 =
          anchor_comp_map.find(name_anchor_map.find(name2)->second)->second;
        pairs.push_back(ParticlePair(comp1,comp2));
      }
    }
  }
  return pairs;
}

void SingleDominoRun::set_restraints(
     const ParticlePairs &anchor_comp_pairs,
     domino::RestraintEvaluatorFromFile *re) {
  IMP_INTERNAL_CHECK(restraints_initialized_==false,
              "The restraints have already been initalized"<<std::endl);
  Restraints rsrs;
  //set pair restraints
  ParticlePairs pairs = get_pairs_of_interacting_components(anchor_comp_pairs);
  //Float pairwise_weight=0.2;
  RestraintFilenameGenerator rg(path_);
  for(ParticlePairs::iterator it = pairs.begin(); it != pairs.end(); it++)
    {
      IMP_LOG(TERSE,"setting pairwise restraint between:"<<
                    it->get(0)->get_name()<< " and " <<
                    it->get(1)->get_name() << std::endl);
      Particles ps;
      ps.push_back(it->get(0));
      ps.push_back(it->get(1));
      /* Restraint *r = multifit::create_weighted_excluded_volume_restraint(
            core::RigidBody(ps[0]),
            core::RigidBody(ps[1]));*/
      core::RigidBodies rbs;
      rbs.push_back(core::RigidBody(ps[0]));
      rbs.push_back(core::RigidBody(ps[1]));
      helper::SimpleExcludedVolume simple_ev =
        helper::create_simple_excluded_volume_on_rigid_bodies(rbs);
      mdl_->add_restraint(simple_ev.get_restraint());
      if (re != NULL) {
        std::string r_fn = rg.get_pairwise_filename(ps[0],ps[1]);
        IMP_LOG(TERSE,"adding restraint file:"<<r_fn<<std::endl);
        re->set_restraint_file(simple_ev.get_restraint(),r_fn.c_str());
      }
      d_opt_->add_restraint(simple_ev.get_restraint(),ps,ev_weight_);
      /*          1./((core::get_leaves(atom::Hierarchy(ps[0]))).size()+
          (core::get_leaves(atom::Hierarchy(ps[1]))).size()));*/
      //        pairwise_weight);
  }
  //set single restraints
  for(atom::Hierarchies::iterator it=components_.begin();
      it != components_.end(); it++) {
    Particle *p = it->get_particle();
    IMP_LOG(TERSE,"setting EM restraint for:"<<
                  p->get_name()<<std::endl);
      Particles ps;
      ps.push_back(p);
    helper::SimpleEMFit simple_fit=
       helper::create_simple_em_fit(*it,dmap_);
    mdl_->add_restraint(simple_fit.get_restraint());
    if (re != NULL) {
      std::string r_fn = rg.get_single_filename(p);
      IMP_LOG(TERSE,"adding restraint file:"<<r_fn<<std::endl);
      re->set_restraint_file(simple_fit.get_restraint(),r_fn.c_str());
    }
    d_opt_->add_restraint(simple_fit.get_restraint(),
                          ps,em_weight_);
  }
  restraints_initialized_=true;
}


domino::DominoOptimizer* SingleDominoRun::optimize(int num_solutions) {
  IMP_LOG(TERSE,"start single domino run optimization"<<std::endl);
  //  IMP_NEW(ConfigurationSet,conf_set,(mdl_,"conf_set.txt"));
  d_opt_->set_sampling_space(&sampler_);
  IMP_LOG(TERSE,"after sampling space setting"<<std::endl);
  d_opt_->set_number_of_solutions(num_solutions);
  IMP_LOG(TERSE,"start domino optimization"<<std::endl);
  d_opt_->optimize(num_solutions);
  IMP_LOG(TERSE,"end domino optimization"<<std::endl);
  /*for(int i=0;i<num_solutions;i++) {
    d_opt_->move_to_opt_comb(i);
    conf_set->save_configuration();
    }*/
  //move back to the original configuration
  Particle *p = new Particle(mdl_);
  algebra::Transformation3D id_t = algebra::get_identity_transformation_3d();
  domino::Transformation::setup_particle(p,id_t);
  for(atom::Hierarchies::iterator it = components_.begin();
      it != components_.end(); it++) {
    tu_.move2state(it->get_particle(),p);
  }
  IMP_LOG(TERSE,"end single domino run optimization"<<std::endl);
  return d_opt_;
}
void SingleDominoRun::setup() {
  IMP_INTERNAL_CHECK(!is_setup_,"the class is already initialized\n");
  IMP_INTERNAL_CHECK(dc_.get_number_of_components()>0,
                     "no components provided"<<std::endl);
  //setup all of the components
  for(int i=0;i<dc_.get_number_of_components();i++) {
    components_.push_back(atom::Hierarchy(dc_.get_component(i)));
  }
  mdl_=components_[0]->get_model();
  //setup the junction tree
  jt_=dc_.get_junction_tree();
  //setup the density map
  dmap_=dc_.get_density_map();
  //setup for sampling space
  tu_=domino::TransformationUtils(components_,true);
  discrete_set_ = new domino::TransformationMappedDiscreteSet
    (new container::ListSingletonContainer(components_));
  sampling_space_initialized_=false;
  restraints_initialized_=false;
  //set the name to anchor mapping
  for(int i=0;i<dc_.get_number_of_density_anchor_points();i++) {
    std::stringstream name;
    name<<i;
    anchor_name_map_[dc_.get_density_anchor_point(i)]=name.str();
  }
  /*
  StringKey name_key("name"); //TODO - have it in a better place, decorator ?
  for(ParticleParticleMapping::iterator it = anchor_comp_map_.begin();
      it != anchor_comp_map_.end();it++) {
    Particle *anchor_point = it->first;
    IMP_INTERNAL_CHECK(anchor_point->has_attribute(name_key),
       "no name has been assigned to the anchor point");
    std::string anchor_point_name = anchor_point->get_value(name_key);
    IMP_INTERNAL_CHECK(name_anchor_map_.find(anchor_point_name) ==
       name_anchor_map_.end(),"the name has already been mapped");
    name_anchor_map_[anchor_point_name]=anchor_point;
  }
  */
  is_setup_=true;
}
SingleDominoRun::SingleDominoRun(
   const DataContainer &dc){
  dc_=dc;
  is_setup_=false;
  d_opt_=NULL;
  setup();
  path_="./";
  ev_weight_ =1.;
  em_weight_=1.;
  d_opt_=NULL;
}
void SingleDominoRun::clear_optimization_data() {
  discrete_set_->clear();
  sampler_.clear();
  if (d_opt_ != NULL) {
    d_opt_=NULL;
  }
  restraints_initialized_=false;
  sampling_space_initialized_=false;
}
domino::DominoOptimizer* SingleDominoRun::run(
        ParticlePairs anchor_comp_pairs,
        Float distance,
        int num_solutions,
        bool init_restraint_values_from_files){
  IMP_INTERNAL_CHECK(is_setup_,"domino run was not setup\n");
  //clear all previous run data
  clear_optimization_data();
  IMP_LOG(TERSE,"SingleDominoRun::run set sampling space\n");
  set_sampling_space(anchor_comp_pairs,distance);
 //is it ok to pass null here ?
  domino::RestraintEvaluatorI *re;
  domino::RestraintEvaluatorFromFile *re_ff;
  if (init_restraint_values_from_files) {
    re_ff = new domino::RestraintEvaluatorFromFile();
    re =re_ff;
  }
  else {
    re = new domino::RestraintEvaluator(&sampler_);
  }
  d_opt_ = new domino::DominoOptimizer(jt_,mdl_,re);
  IMP_LOG(TERSE,"SingleDominoRun::run set restraints\n");
  if (init_restraint_values_from_files) {
  IMP_LOG(TERSE,
          "SingleDominoRun::run the restraints are calcualted from files\n");
    set_restraints(anchor_comp_pairs,re_ff);
  }
  else {
  IMP_LOG(TERSE,
          "SingleDominoRun::run the restraints are calcualted in run time\n");
    set_restraints(anchor_comp_pairs);
  }
  IMP_IF_LOG(VERBOSE) {
    if (init_restraint_values_from_files) {
      IMP_LOG(VERBOSE,"the restraint evaluator: ");
      IMP_LOG_WRITE(VERBOSE,re_ff->show());
    }
  }
  IMP_LOG(TERSE,"SingleDominoRun::run optimize\n");
  return optimize(num_solutions);
  //ConfigurationSet *conf_set = optimize();
  //IMP_LOG(TERSE,"SingleDominoRun::run done!\n");
  //return conf_set;
}
void SingleDominoRun::set_sampling_space(
  const ParticlePairs &anchor_comp_pairs,Float distance) {
  std::cout<<"uing distance:"<<distance<<std::endl;
  IMP_INTERNAL_CHECK(sampling_space_initialized_==false,
            "the sampling space has already been set"<<std::endl);
  IntKey orig_ind_key = IntKey("orig_ind");
  //add valid transformation for each rigid body
  for(ParticlePairs::const_iterator it = anchor_comp_pairs.begin();
      it != anchor_comp_pairs.end(); it++){
    //the component
    Particle *comp = it->get(1);
    //the mapped anchor point
    Particle *mapped_ap = it->get(0);
    IMP_LOG(VERBOSE,"set sampling space for particle:"<<
            comp->get_name()<<" which is mapped to:"<<
            mapped_ap->get_name()<<std::endl);
    //set the domino name attribute to the mapped anchor point
    comp->set_value(domino::node_name_key(),
                    anchor_name_map_[mapped_ap]);
    //the subset of fitting solutions close to the mapped ap
    IMP_LOG(VERBOSE,"getting fits"<<std::endl);
    FittingSolutionRecords ts_pruned =
      get_close_to_point(dc_.get_fitting_solutions(comp),
                         atom::Hierarchy(comp),
                         mapped_ap,distance);
    IMP_LOG(VERBOSE,"number of fits for comp:"<<
            comp->get_name()<<" is:"<<ts_pruned.size()<<
            " out of:"<<dc_.get_fitting_solutions(comp).size()<<std::endl)
      std::cout<<"number of fits for comp:"<<
        comp->get_name()<<" is:"<<ts_pruned.size()<<
        " out of:"<<dc_.get_fitting_solutions(comp).size()<<std::endl;
    //add these fitting solutions to the sampling space
    for(unsigned int i=0;i<ts_pruned.size();i++) {
      Particle *state_p = new Particle(mdl_);
      domino::Transformation::setup_particle(
        state_p,ts_pruned[i].get_transformation(),i);
      state_p->add_attribute(orig_ind_key,i);//TODO - why is it needed?
      discrete_set_->add_mapped_state(comp,state_p);
    }
    IMP_LOG(VERBOSE,"end setting sampling space for particle"<<std::endl);
  }
  sampler_ = domino::TransformationCartesianProductSampler(
               discrete_set_,components_,true);
  sampling_space_initialized_=true;
}

IMPMULTIFIT_END_NAMESPACE
