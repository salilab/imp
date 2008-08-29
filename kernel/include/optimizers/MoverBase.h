/**
 *  \file MoverBase.h    \brief A class to help implement movers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MOVER_BASE_H
#define __IMP_MOVER_BASE_H

#include "Mover.h"
#include "../Particle.h"
#include "../macros.h"

#include <vector>

namespace IMP
{

//! A class to help implement movers
/** This class helps in implementing Movers by allowing changes to be easily
    rolled back. It maintains a list of particles and a list of attributes.
    All changes to the product of those two lists will be rolled back
    when reject_move() is called.

    See NormalMover for a simple example using this class.
 */
class IMPDLLEXPORT MoverBase: public Mover
{
  std::vector<Floats> floats_;
  std::vector<Ints> ints_;
public:
  virtual void accept_move(){}
  virtual void reject_move();

  /** This sets everything up and then calls the generate_move method.
   */
  virtual void propose_move(float f);

  IMP_LIST(public, Particle, particle, Particle*);
  IMP_LIST(public, FloatKey, float_key, FloatKey);
  IMP_LIST(public, IntKey, int_key, IntKey);

protected:
  //! implement this method to propose a move
  /** See NormalMover for a simple example.
   */
  virtual void generate_move(float f)=0;

  //! Get the value of a controlled attribute
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Float get_float(unsigned int i, unsigned int j) const {
    IMP_assert(get_number_of_particles() == floats_.size(),
               "Only call get_float from within generate_proposal");
    return get_particle(i)->get_value(get_float_key(j));
  }

  //! Get an int attribute value
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Int get_int(unsigned int i, unsigned int j) const {
    IMP_assert(get_number_of_particles() == ints_.size(),
               "Only call get_int from within generate_proposal");
    return get_particle(i)->get_value(get_int_key(j));
  }

  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Float t) {
    if (get_particle(i)->get_is_optimized(get_float_key(j))) {
      get_particle(i)->set_value(get_float_key(j), t);
    }
  }
  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Int t) {
    get_particle(i)->set_value(get_int_key(j), t);
  }

  MoverBase(){}
  ~MoverBase(){}
};

} // namespace IMP

#endif  /* __IMP_MOVER_BASE_H */
