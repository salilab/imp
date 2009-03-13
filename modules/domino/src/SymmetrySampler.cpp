/**
 * \file  SymmetrySampler.cpp
 * \brief Sample transformations of particles while preseving N-symmetry.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/domino/SymmetrySampler.h>
#include <IMP/atom/utilities.h>
#include <IMP/atom/pdb.h>

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
}
void SymmetrySampler::set_symmetry(const algebra::Cylinder3D &c) {
  cyl_=c;
}

void SymmetrySampler::populate_states_of_particles(Particles *particles,
                                                   Combinations *states) const {
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
}

void SymmetrySampler::reset_placement(const CombState *cs) {
  Particle *p;
  for (CombData::const_iterator it = cs->get_data()->begin();
        it != cs->get_data()->end(); it++) {
    p = it->first;
  atom::copy_atom_positions(atom::MolecularHierarchyDecorator::cast(ref_),
                            atom::MolecularHierarchyDecorator::cast(p));
  }
}

//! Set the attributes of the particles in the combination to the states
//! indicated in the combination
void SymmetrySampler::move2state(const CombState *cs) {

  //first move the atoms to their initial location
  reset_placement(cs);
  std::cout<<"in SymmetrySampler::move2state " << std::endl;
  Particle *p;
  algebra::Transformation3D t;
  for (std::map<Particle *,unsigned int>::const_iterator
         it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p = it->first;
    t = ts_->get_transformation(it->second);
    t.show();
    double angle = 2.*PI/ps_->size()*symm_deg_[p];
    std::cout<<"angle : " << angle <<std::endl;
    std::cout<<"going to transform " << std::endl;
    std::cout<<"going to transform : " << cyl_.get_length() << std::endl;
    algebra::rotate(cyl_,it->second).show();
    std::cout<<"going to transform " << std::endl;
    atom::transform(atom::MolecularHierarchyDecorator::cast(p),
                    algebra::rotate(cyl_,angle).compose(t));
                    //t.compose(algebra::rotate(cyl_,angle)));
    std::stringstream name;
    name<<p->get_value(StringKey("name"))<<"__"<<cs->key()<<".pdb";
    atom::write_pdb(atom::MolecularHierarchyDecorator::cast(p),name.str());
  }
  std::cout<<"in SymmetrySampler::move2state END" << std::endl;
}

IMPDOMINO_END_NAMESPACE
