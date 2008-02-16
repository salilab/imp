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
  FloatKeys fkeys_;
  IntKeys ikeys_;
  Particles ps_;
public:
  virtual void accept_move(){}
  virtual void reject_move();

  /** This sets everything up and then calls the generate_move method.
   */
  virtual void propose_move(float f);

protected:
  //! implement this method to propose a move
  /** See NormalMover for a simple example.
   */
  virtual void generate_move(float f)=0;

  //! Add to the set of particles being controlled
  void add_particles(const Particles &ps) {
    ps_.insert(ps_.end(), ps.begin(), ps.end());
  }
  //! Clear the set of particles being controlled
  void clear_particles() {
    ps_.clear();
  }

  //! Get the number of particles
  unsigned int number_of_particles() const {
    return ps_.size();
  }
  Particle *get_particle(unsigned int i) const {
    IMP_check(i< ps_.size(), "Bad index in MoverBase::get_particle",
              IndexException("Particle index out of range in mover base"));
    return ps_[i];
  }

  //! Add an attribute
  unsigned int add_key(FloatKey k) {
    fkeys_.push_back(k); 
    return fkeys_.size()-1;
  }
  //! Add an attribute
  unsigned int add_key(IntKey k) {
    ikeys_.push_back(k); 
    return ikeys_.size()-1;
  }

  unsigned int number_of_float_keys() const {return fkeys_.size();}
  unsigned int number_of_int_keys() const {return ikeys_.size();}



  //! Get the value of a controlled attribute
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Float get_float(unsigned int i, unsigned int j) const {
    IMP_assert(i < ps_.size(),
               "Out of range controlled attribute in guard");
    IMP_assert(j < fkeys_.size(),
               "Out of range controlled attribute in guard");
    IMP_assert(ps_.size() == floats_.size(),
               "Only call get_float from within generate_proposal");
    return get_particle(i)->get_value(fkeys_[j]);
  }

  //! Get an int attribute value
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Int get_int(unsigned int i, unsigned int j) const {
    IMP_assert(i < ps_.size(),
               "Out of range controlled attribute in guard");
    IMP_assert(j < ikeys_.size(),
               "Out of range controlled attribute in guard");
    IMP_assert(ps_.size() == ints_.size(),
               "Only call get_float from within generate_proposal");

    return get_particle(i)->get_value(ikeys_[j]);
  }
  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Float t) {
    if (get_particle(i)->get_is_optimized(fkeys_[j])) {
      get_particle(i)->set_value(fkeys_[j], t);
    }
  }
  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Int t) {
    get_particle(i)->set_value(ikeys_[j], t);
  }

  MoverBase(){}
  ~MoverBase(){}
};

} // namespace IMP

#endif  /* __IMP_MOVER_BASE_H */
