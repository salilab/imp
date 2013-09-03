/**
 *  \file isd/Weight.cpp
 *  \brief Add a name to a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Weight.h>

IMPISD_BEGIN_NAMESPACE

void Weight::do_setup_particle(kernel::Model *m,
                               kernel::ParticleIndex pi){
  m->add_attribute(get_nstates_key(), pi, 0);
  for(int i=0; i<nstates_max; ++i){
    m->add_attribute(get_weight_key(i), pi, 0.0);
  }
}

 IntKey Weight::get_nstates_key() {
  static IntKey k("nstates");
  return k;
 }

 FloatKey Weight::get_weight_key(int j) {
  static FloatKeys kk;
  if(kk.empty()){
   for(int i=0; i<nstates_max; ++i){
    std::stringstream out; out << i;
    kk.push_back(FloatKey("weight"+out.str()));
   }
  }
  return kk[j];
 }

//! Set all the weights
 void Weight::set_weights(algebra::VectorKD w){
   IMP_USAGE_CHECK(static_cast<int>(w.get_dimension())
                   == get_number_of_states(), "Out of range");
  for(int i=0; i<get_number_of_states(); ++i){
   get_particle()->set_value(get_weight_key(i),w[i]);
  }
 }

//! Add one weight
 void Weight::add_weight() {
   int i = get_particle()->get_value(get_nstates_key());
   IMP_USAGE_CHECK(i < nstates_max, "Out of range");
   get_particle()->set_value(get_nstates_key(),i+1);
   Float w = 1.0/static_cast<Float>(get_number_of_states());
   for(int i=0;i<get_number_of_states();++i){
     get_particle()->set_value(get_weight_key(i),w);
   }
 }

//! Get the i-th weight
 Float Weight::get_weight(int i) {
   IMP_USAGE_CHECK(i < get_number_of_states(), "Out of range");
   return get_particle()->get_value(get_weight_key(i));
 }

//! Get all weights
 algebra::VectorKD Weight::get_weights() {
   Floats ww;
   for(int i=0;i<get_number_of_states();++i){ww.push_back(get_weight(i));}
   return algebra::VectorKD(ww);
 }

//! Set weights are optimized
 void Weight::set_weights_are_optimized(bool tf) {
   for(int i=0;i<nstates_max;++i){
    get_particle()->set_is_optimized(get_weight_key(i),tf);
   }
 }

//! Get number of states
 Int Weight::get_number_of_states() {
   return get_particle()->get_value(get_nstates_key());
 }

 void Weight::show(std::ostream &out) const {
   out << "Weight ";
 }

IMPISD_END_NAMESPACE
