/**
 *  \file internal/particle_save.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_PARTICLE_SAVE_H
#define IMP_INTERNAL_PARTICLE_SAVE_H

#include "../Particle.h"
#include <vector>


IMP_BEGIN_INTERNAL_NAMESPACE
class IMPEXPORT ParticleData {
  friend class ParticleDiff;
  typedef ArrayStorage<FloatAttributeTableTraits> FloatTable;
  typedef ArrayStorage<IntAttributeTableTraits> IntTable;
  typedef ArrayStorage<StringAttributeTableTraits> StringTable;
  typedef RefCountedStorage<ParticlesAttributeTableTraits> ParticleTable;
  typedef RefCountedStorage<ObjectsAttributeTableTraits> ObjectTable;
  FloatTable floats_;
  IntTable ints_;
  StringTable strings_;
  ParticleTable particles_;
  ObjectTable objects_;
public:
  ParticleData(){}
  //! Store the data from the particle
  ParticleData(Particle *p);
  //! overwrite the particle
  void apply(Particle *p) const;
};

class IMPEXPORT ParticleDiff {
  typedef std::pair<FloatKey, Float> FloatPair;
  typedef std::pair<IntKey, Int> IntPair;
  typedef std::pair<StringKey, String> StringPair;
  typedef std::pair<ParticleKey, Pointer<Particle> > ParticlePair;
  typedef std::pair<ObjectKey, Pointer<Object> > ObjectPair;
  std::vector<FloatPair> floats_a_;
  std::vector<FloatKey> floats_r_;
  std::vector<IntPair> ints_a_;
  std::vector<IntKey> ints_r_;
  std::vector<StringPair> strings_a_;
  std::vector<StringKey> strings_r_;
  std::vector<ParticlePair> particles_a_;
  std::vector<ParticleKey> particles_r_;
  std::vector<ObjectPair> objects_a_;
  std::vector<ObjectKey> objects_r_;
 public:
  ParticleDiff(){}
  //! Compute the diff between the two
  ParticleDiff(const ParticleData &base, Particle *p);
  void apply(Particle *p) const;
};


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_PARTICLE_SAVE_H */
