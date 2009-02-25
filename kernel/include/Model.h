/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_MODEL_H
#define IMP_MODEL_H

#include "config.h"
#include "Object.h"
#include "Particle.h"
#include "internal/kernel_version_info.h"
#include "container_macros.h"
#include "base_types.h"
#include "VersionInfo.h"

#include <limits>

IMP_BEGIN_NAMESPACE

class Particle;
class Restraint;
class ScoreState;
typedef std::vector<Restraint*> Restraints;
typedef std::vector<ScoreState*> ScoreStates;

//! Class for storing model, its restraints, and particles.
/** The Model maintains a standard IMP container for each of Particle,
    ScoreState and Restraint object types.

    \note Think carefully about using the iterators over the entire set
          of Particles or Restraints. Most operations should be done using
          a user-passed set of Particles or Restraints instead.
 */
class IMPEXPORT Model: public Object
{
private:
  friend class Restraint;
  friend class Particle;
  typedef Particle::Storage ParticleStorage;

  unsigned int iteration_;
  ParticleStorage particles_;
  unsigned int last_particle_index_;
  std::map<FloatKey, FloatPair> ranges_;

  void add_particle_internal(Particle *p) {
    particles_.push_back(p);
    p->iterator_= --particles_.end();
    p->model_= this;
    internal::ref(p);
    std::ostringstream oss;
    oss << "P" << ++last_particle_index_;
    p->set_name(oss.str());
    p->index_= last_particle_index_;
  }
public:
  /** Construct an empty model */
  Model();
  /** Particles, ScoreStates and Restraints are ref-counted so they
      will be deleted if no other Pointers to them are held. */
  ~Model();

  IMP_LIST(public, ScoreState, score_state, ScoreState*);
  IMP_LIST(public, Restraint, restraint, Restraint*);
 public:


  //! Methods to manipulate particles
  //@{
  //! Remove the particle from this model
  /** Since particles are ref counted the object will still
      be valid until all references are removed.*/
  void remove_particle(Particle *p) {
    IMP_check(p->get_model() == this,
              "The particle does not belong to this model",
              ValueException);
    particles_.erase(p->iterator_);
    internal::unref(p);
    p->model_=NULL;
  }
  /** \note This really should only be used for debugging and only
      then if you really know what you are doing as the number of
      Particles can change unexpectedly.
   */
  unsigned int get_number_of_particles() const {
    return particles_.size();
  }
  //! Think before using...
  /** \note Only use this if you really know what you are doing as
      Particles can be added to the object from many different places.
  */
  typedef ParticleStorage::iterator ParticleIterator;
  //! Iterate through the particles
  ParticleIterator particles_begin() {
    return particles_.begin();
  }
  //! Iterate through the particles
  ParticleIterator particles_end() {
    return particles_.end();
  }
  typedef ParticleStorage::const_iterator ParticleConstIterator;
  ParticleConstIterator particles_begin() const {
    return particles_.begin();
  }
  ParticleConstIterator particles_end() const {
    return particles_.end();
  }
  //! \deprecated Use the Particle(Model*) constructor
  void add_particle(Particle *p) {
    add_particle_internal(p);
  }
  //@}

  Particle* get_particle(unsigned int i) const {
    static bool printed=false;
    if (!printed) {
      IMP_WARN("DO NOT USE Model::get_particle(unsigned int)"
               << " it is extremely slow and going away.");
      printed=true;
    }
    for (ParticleConstIterator it = particles_begin();
         it != particles_end(); ++it) {
      if ((*it)->get_index() ==i) {
        return *it;
      }
    }
    throw IndexException("Bad particle index");
  }

  //! Return a range for the attribute.
  /** This range is either the range found in the current set of particles
      or a user-set range (using set_range). This method may be linear, the
      first time it is called after each evaluate call.
   */
  FloatPair get_range(FloatKey k) const;

  //! Set the expected range for an attribute
  /** This range is not enforced, it is just to help the optimizer. */
  void set_range(FloatKey k, FloatPair range) {
    ranges_[k]=range;
  }

  //! Evaluate all of the restraints in the model and return the score.
  /** \param[in] calc_derivs If true, also evaluate the first derivatives.
      \return The score.

      All of the stored ScoreState objects are updated before the
      restraints are evaluated.
   */
  Float evaluate(bool calc_derivs);

  //! Show the model contents.
  /** \param[in] out Stream to write model description to.
   */
  void show (std::ostream& out = std::cout) const;

  //! \return version and authorship information.
  VersionInfo get_version_info() const {
    return internal::kernel_version_info;
  }
};

IMP_OUTPUT_OPERATOR(Model);

IMP_END_NAMESPACE

#endif  /* IMP_MODEL_H */
