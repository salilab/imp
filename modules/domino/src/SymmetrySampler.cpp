/**
 * \file  SymmetrySampler.cpp
 * \brief Sample transformations of particles while preseving N-symmetry.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include <IMP/domino/SymmetrySampler.h>
#include <IMP/atom/pdb.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/core/Transform.h>
IMPDOMINO_BEGIN_NAMESPACE
//  virtual void show(std::ostream& out = std::cout) const {}

//   //! Show the sampling space of a single particle
//   virtual void show_space(Particle *p,
//                       std::ostream& out = std::cout) const {}
SymmetrySampler::SymmetrySampler(Particles *ps,
                                 TransformationDiscreteSet *ts,
                                 const algebra::Cylinder3D &c) : cyl_(c) {
  ps_=ps;
  ts_=ts;
  for(unsigned int i=0;i<ps_->size();i++){
    symm_deg_[(*ps_)[i]]=i;
  }
  //superpose the particles on the first one and use that as reference
  ref_[(*ps_)[0]]=algebra::identity_transformation();
  algebra::Vector3Ds ref_positions;
  Particles ps1 =
    atom::get_by_type((*ps_)[0], atom::MolecularHierarchyDecorator::ATOM);

  for(Particles::iterator it=ps1.begin();it!=ps1.end();it++) {
    ref_positions.push_back(core::XYZDecorator::cast(*it).get_coordinates());
  }

  for(unsigned int i=1;i<ps_->size();i++) {
    algebra::Vector3Ds other_positions;
    Particles ps2 =
      atom::get_by_type((*ps_)[i], atom::MolecularHierarchyDecorator::ATOM);
    for(Particles::iterator it=ps2.begin();it!=ps2.end();it++) {
      other_positions.push_back(
         core::XYZDecorator::cast(*it).get_coordinates());
    }
    ref_[(*ps_)[i]]=
      algebra::rigid_align_first_to_second(other_positions,ref_positions);
  }
}

void SymmetrySampler::populate_states_of_particles(Particles *particles,
                                                   Combinations *states) const {
  IMP_assert(states != NULL,"the states should be initialized");
  IMP_LOG(VERBOSE,"SymmetrySampler:: start populaing states of particles");
  std::cout<<"SymmetrySampler:: start populaing states of particles"<<std::endl;
  CombState *calc_state;
  int comb_size = particles->size();
  std::vector<int> v_int(ts_->get_number_of_states());
  IMP_LOG(VERBOSE, "Combination size: " << comb_size <<
          " number of states: " << ts_->get_number_of_states());
  for (unsigned int i = 0; i < ts_->get_number_of_states(); ++i) {
    calc_state = new CombState();
    for (int j = 0; j < comb_size; j++) {
      calc_state->add_data_item((*particles)[j],i);
    }
    (*states)[calc_state->partial_key(particles)]=calc_state;
  }
  IMP_LOG(VERBOSE,
          "SymmetrySampler:: end populaing states of particles"<<std::endl);
}
//TODO - consider keeping particles and not using get_leaves
void SymmetrySampler::reset_placement(const CombState *cs) {
  IMP_LOG(VERBOSE,"SymmetrySampler:: start reset placement"<<std::endl);
  Particle *p;
  for (CombData::const_iterator it = cs->get_data()->begin();
        it != cs->get_data()->end(); it++) {
    p = it->first;
    IMP_LOG_WRITE(VERBOSE,p->show());
    apply(make_pointer(new core::Transform(ref_[p])),
          core::get_leaves(atom::MolecularHierarchyDecorator::cast(p)));
    IMP_LOG(VERBOSE,"end loop iteration"<<std::endl);
  }
  IMP_LOG(VERBOSE,"SymmetrySampler:: end reset placement"<<std::endl);
}

//! Set the attributes of the particles in the combination to the states
//! indicated in the combination
void SymmetrySampler::move2state(const CombState *cs) {
  IMP_LOG(VERBOSE,"SymmetrySampler:: start moving to state"<<std::endl);
  //first move the atoms to their initial location
  reset_placement(cs);
  Particle *p;
  algebra::Transformation3D t;
  for (std::map<Particle *,unsigned int>::const_iterator
         it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p = it->first;
    t = ts_->get_transformation(it->second);
    double angle = 2.*PI/ps_->size()*symm_deg_[p];
    // was algebra::rotate(cyl_,angle).compose(t)
    algebra::Transformation3D tr
      =compose(algebra::rotation_in_radians_about_axis(cyl_.get_direction(),
                                                       angle),t);
    apply(make_pointer(new core::Transform(tr)),
          core::get_leaves(atom::MolecularHierarchyDecorator::cast(p)));
    ref_[p]= compose(algebra::rotation_in_radians_about_axis(
                     cyl_.get_direction(),angle),t).get_inverse();
 //    std::stringstream name;
//     name<<p->get_value(StringKey("name"))<<"__"<<cs->key()<<".pdb";
//     atom::write_pdb(atom::MolecularHierarchyDecorator::cast(p),name.str());
  }
  IMP_LOG(VERBOSE,"SymmetrySampler:: end moving to state"<<std::endl);
}
IMPDOMINO_END_NAMESPACE
