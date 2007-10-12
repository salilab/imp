/*
 *  Complexes_Restraints.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_COMPLEXES_RESTRAINTS_H
#define __IMP_COMPLEXES_RESTRAINTS_H

#include <list>

#include "../IMP_config.h"
#include "../emfile.h"
#include "Restraint.h"

namespace imp
{

// Restrict particle position with respect to one or more coordinates
class IMPDLLEXPORT CoordinateRestraint : public Restraint
{
public:
  CoordinateRestraint(Model& model,
                 Particle* p1,
                 const std::string type,
                 BasicScoreFuncParams* score_func_params);
  virtual ~CoordinateRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  // variables used to determine the distance
  FloatIndex x1_, y1_, z1_;
  // type of coordinate position to use:
  // X_AXIS, Y_AXIS, Z_AXIS, XY_RADIAL, XZ_RADIAL, YZ_RADIAL, XYZ_SPHERE
  std::string axis_;
  // math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};

// Restrict particle position to interior of a torus
class IMPDLLEXPORT TorusRestraint : public Restraint
{
public:
  TorusRestraint(Model& model,
            Particle* p1,
            const Float main_radius,
            const Float tube_radius,
            BasicScoreFuncParams* score_func_params);
  virtual ~TorusRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  // variables used to determine the distance
  FloatIndex x1_, y1_, z1_;
  // main radius of the torus
  Float main_radius_;
  // radius of the torus tube
  Float tube_radius_;
  // math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};


// Restrict maximum distance between any two particles
class IMPDLLEXPORT ProximityRestraint : public Restraint
{
public:
  ProximityRestraint(Model& model,
                // couldn't get Swig to work with std::vector<Particle*>&
                std::vector<int>& particle_indexes,
                const Float distance,
                BasicScoreFuncParams* score_func_params);

  ProximityRestraint(Model& model,
                // couldn't get Swig to work with std::vector<Particle*>&
                std::vector<int>& particle_indexes,
                const std::string attr_name,
                const Float distance,
                BasicScoreFuncParams* score_func_params);

  virtual ~ProximityRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle_indexes);

  /** number of particles in the restraint */
  int num_particles_;

  /** total number of restraints being tested */
  int num_restraints_;
  /** calculated restraint scores */
  std::vector<Float> scores_;
  /** indexes of local indexes sorted by energies */
  std::vector<int> rsr_idx_;
  /** the distance restraints */
  std::vector<DistanceRestraint*> dist_rsrs_;
};

// Restrict max distance between at one or more pair of particles of any two sets of particles
class IMPDLLEXPORT PairConnectivityRestraint : public Restraint
{
public:
  PairConnectivityRestraint(Model& model,
                        // couldn't get Swig to work with std::vector<Particle*>&
                        std::vector<int>& particle1_indexes,
                        std::vector<int>& particle2_indexes,
                        BasicScoreFuncParams* score_func_params,
                        const int num_to_apply = 1,
                        const bool particle_reuse = false);

  PairConnectivityRestraint(Model& model,
                        // couldn't get Swig to work with std::vector<Particle*>&
                        std::vector<int>& particle1_indexes,
                        std::vector<int>& particle2_indexes,
                        const std::string attr_name,
                        BasicScoreFuncParams* score_func_params,
                        const int num_to_apply,
                        const bool particle_reuse = false);

  virtual ~PairConnectivityRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  class RestraintScore
  {
  public:
    RestraintScore() {}
    ~RestraintScore() {}
    void evaluate(void) {
      score_ = rsr_->evaluate(false);
    }
    bool operator<(const RestraintScore& rs) const {
      return score_ < rs.score_;
    }

    int part1_idx_;
    int part2_idx_;
    DistanceRestraint* rsr_;
    Float score_;
  };

  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle1_indexes,
              std::vector<int>& particle2_indexes);

  // variables to determine the particle type
  std::vector<IntIndex> type_;

  /** number of particles all together */
  int num_particles_;
  /** number of particles in vector 1 */
  int num_particles1_;
  /** number of particles in vector 2 */
  int num_particles2_;
  /** total number of restraints to apply */
  int num_to_apply_;
  /** true if a particle can be involved in more than one restraint that is applied */
  int particle_reuse_;
  /** which of particles have already been used */
  std::vector<bool> used_;

  /** total number of restraints being tested */
  int num_restraints_;
  /** restraints and their scores */
  std::list<RestraintScore> rsr_scores_;
};


// Restrict max distance between at least one pair of particles of any two distinct types
class IMPDLLEXPORT ConnectivityRestraint : public Restraint
{
public:
  ConnectivityRestraint(Model& model,
                   // couldn't get Swig to work with std::vector<Particle*>&
                   std::vector<int>& particle_indexes,
                   const std::string type,
                   BasicScoreFuncParams* score_func_params);

  ConnectivityRestraint(Model& model,
                   // couldn't get Swig to work with std::vector<Particle*>&
                   std::vector<int>& particle_indexes,
                   const std::string type,
                   const std::string attr_name,
                   BasicScoreFuncParams* score_func_params);

  virtual ~ConnectivityRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  // switch to using rsr_scores to allow STL sorting
  class RestraintScore
  {
  public:
    RestraintScore() {}
    ~RestraintScore() {}
    void evaluate(void) {
      score_ = rsr_->evaluate(false);
    }
    bool operator<(const RestraintScore& rs) const {
      return score_ < rs.score_;
    }

    int part1_type_;
    int part2_type_;
    DistanceRestraint* rsr_;
    Float score_;
  };

  /** restraints and their scores */
  std::list<RestraintScore> rsr_scores_;

  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle_indexes,
              const std::string type);

  // variables to determine the particle type
  std::vector<IntIndex> type_;

  /** number of particles in the restraint */
  int num_particles_;

  /** maximum type (type can be from 0 to max_type-1) */
  int max_type_;
  /** number of particle types */
  int num_types_;
  /** particle types */
  std::vector<int> particle_type_;

  /** total number of restraints being tested */
  int num_restraints_;

  /** each unconnected tree has a non-zero id */
  std::vector<int> tree_id_;
};


// Restrict min distance between all pairs of particles of formed from one or two sets of particles.
// If two sets of particles are passed, it assumes that the two sets of particles have no overlap
// (otherwise, you will get repeats).
class IMPDLLEXPORT ExclusionVolumeRestraint : public Restraint
{
public:
  ExclusionVolumeRestraint(Model& model,
                       // couldn't get Swig to work with std::vector<Particle*>&
                       std::vector<int>& particle1_indexes,
                       std::vector<int>& particle2_indexes,
                       const std::string attr_name,
                       BasicScoreFuncParams* score_func_params
                      );

  ExclusionVolumeRestraint(Model& model,
                       // couldn't get Swig to work with std::vector<Particle*>&
                       std::vector<int>& particle_indexes,
                       const std::string attr_name,
                       BasicScoreFuncParams* score_func_params
                      );

  virtual ~ExclusionVolumeRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  /** number of particles all together */
  int num_particles_;
  /** number of particles in vector 1 */
  int num_particles1_;
  /** number of particles in vector 2 */
  int num_particles2_;

  /** total number of restraints */
  int num_restraints_;
  /** restraints and their scores */
  std::vector<DistanceRestraint*> dist_rsrs_;
};

// Calculate score based on fit to EM map
class IMPDLLEXPORT CoarseEMRestraint : public Restraint
{
public:
  CoarseEMRestraint(Model& model,
                std::vector<int>& particle_indexes,
                EM_Density *emdens,
                int nx,
                int ny,
                int nz,
                float pixelsize,
                float resolution,
                std::string radius_str,
                std::string weight_str,
                EM_Gridcoord *gridcd,
                float scalefac
               );

  virtual ~CoarseEMRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.0.1";
  }
  virtual std::string last_modified_by(void) const {
    return "Frido and Keren";
  }

protected:
  EM_Density *emdens_;
  int nx_;
  int ny_;
  int nz_;
  float pixelsize_;
  float resolution_;
  EM_Gridcoord *gridcd_;
  float scalefac_;

  int num_particles_;

  // weight and radius associated with each particle
  Float *radius_;
  Float *weight_;

  // coordinates to pass to EM score C routine
  FloatIndex *x_;
  FloatIndex *y_;
  FloatIndex *z_;

  // coordinates to pass to EM score C routine
  Float *cdx_;
  Float *cdy_;
  Float *cdz_;

  // partial derivs to pass to EM score C routine
  Float *dvx_;
  Float *dvy_;
  Float *dvz_;
};

} // namespace imp

#endif /* __IMP_COMPLEXES_RESTRAINTS_H */
