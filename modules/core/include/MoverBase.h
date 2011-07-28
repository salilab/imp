/**
 *  \file MoverBase.h    \brief A class to help implement movers.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MOVER_BASE_H
#define IMPCORE_MOVER_BASE_H

#include "core_config.h"
#include "Mover.h"

#include <IMP/internal/container_helpers.h>
#include <IMP/macros.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! A class to help implement movers
/** This class helps in implementing Movers by allowing changes to be easily
    rolled back. It maintains a list of particles and a list of attributes.
    All changes to the product of those two lists will be rolled back
    when reject_move() is called.

    See NormalMover for a simple example using this class.
 */
class IMPCOREEXPORT MoverBase: public Mover
{
  std::vector<std::vector<Float> > values_;
  std::vector<FloatKey> keys_;
  Model *m_;
  ParticleIndexes particles_;
public:
  virtual void reset_move();

  /** This sets everything up and then calls the generate_move method.
   */
  virtual void propose_move(Float f);

protected:
  unsigned int get_number_of_particles() const {
    return particles_.size();
  }
  unsigned int get_number_of_keys() const {
    return keys_.size();
  }

  //! implement this method to propose a move
  /** See NormalMover for a simple example.
   */
  virtual void do_move(Float f)=0;

  //! Get the value of a controlled attribute
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Float get_value(unsigned int i, unsigned int j) const {
    IMP_USAGE_CHECK(j < keys_.size(), "Out of range key");
    IMP_USAGE_CHECK(i < particles_.size(), "Out of range particle");
    return m_->get_attribute(keys_[j], particles_[i]);
  }

  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Float t) {
    IMP_USAGE_CHECK(j < keys_.size(), "Out of range key");
    IMP_USAGE_CHECK(i < particles_.size(), "Out of range particle");
    if (m_->get_is_optimized(keys_[j], particles_[i])) {
      m_->set_attribute(keys_[j], particles_[i], t);
    }
  }

  MoverBase(const ParticlesTemp &ps,
            const FloatKeys &keys,
            std::string name): Mover(name),
    keys_(keys), m_(IMP::internal::get_model(ps)),
    particles_(IMP::internal::get_index(ps)) {}
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(MoverBase);
};


inline void MoverBase::propose_move(Float f)
{
  values_.resize(particles_.size(),
                 std::vector<Float>(keys_.size(), 0));
  for (unsigned int i=0; i< particles_.size(); ++i) {
    for (unsigned int j=0; j< keys_.size(); ++j) {
      values_[i][j]= get_value(i,j);
    }
  }
  do_move(f);
}


inline void MoverBase::reset_move()
{
  for (unsigned int i=0; i< particles_.size(); ++i) {
    for (unsigned int j=0; j< keys_.size(); ++j) {
      m_->set_attribute(keys_[j], particles_[i], values_[i][j]);
    }
  }
}



IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MOVER_BASE_H */
