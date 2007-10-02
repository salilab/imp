/*
 *  Restraint.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#if !defined(__restraint_h)
#define __restraint_h 1

#include "IMP_config.h"

namespace imp
{

// Abstract class for representing restraints
class IMPDLLEXPORT Restraint
{
public:
  Restraint();
  virtual ~Restraint();
  void set_model_data(Model_Data* model_data);

  // return the score for this restraint or set of restraints
  // ... given the current state of the model
  virtual Float evaluate(bool calc_deriv) = 0;

  // status
  void set_is_active(const bool active);
  bool is_active(void);

  // called when at least one particle has been inactivated
  virtual void check_particles_active(void);

  virtual void show(std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "unknown";
  }
  virtual std::string last_modified_by(void) {
    return "unknown";
  }

protected:
  // all of the particle data
  Model_Data* model_data_;

  // restraint is active if active_ AND particles_active_
  // true if restraint has not been deactivated
  // if it is not active, evaluate should not be called
  bool is_active_;
  // true if all particles that restraint uses are active
  bool are_particles_active_;

  // shouldn't be necessary, but keep around for debugging
  std::vector<Particle*> particles_;
};

// Distance restraint between two particles
class IMPDLLEXPORT RSR_Distance : public Restraint
{
public:
  // particles must be at least this far apart to calculate the
  // ... distance restraint. Force it otherwise.
  static const Float MIN_DISTANCE;

  RSR_Distance(Model& model,
               Particle* p1,
               Particle* p2,
               const Float mean,
               const Float sd,
               Score_Func* score_func);
  RSR_Distance(Model& model,
               Particle* p1,
               Particle* p2,
               const std::string attr_name,
               const Float sd,
               Score_Func* score_func);
  virtual ~RSR_Distance();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  void set_up(Model& model,
              Particle* p1,
              Particle* p2,
              const Float mean,
              const Float sd,
              Score_Func* score_func);

  // variables used to determine the distance
  Float_Index x1_, y1_, z1_, x2_, y2_, z2_;

  // variables used to calculate the math form
  Float mean_, sd_;
  // math form for this restraint (typically one of the harmonics)
  Score_Func* score_func_;
};

// Swig is not accepting the subclassing if it is in a different file
// Restrict particle position with respect to one or more coordinates
class IMPDLLEXPORT RSR_Coordinate : public Restraint
{
public:
  RSR_Coordinate(Model& model,
                 Particle* p1,
                 const std::string type,
                 const Float distance,
                 const Float sd,
                 Score_Func* score_func);
  virtual ~RSR_Coordinate();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  // variables used to determine the distance
  Float_Index x1_, y1_, z1_;
  // type of coordinate position to use:
  // X_AXIS, Y_AXIS, Z_AXIS, XY_RADIAL, XZ_RADIAL, YZ_RADIAL, XYZ_SPHERE
  std::string axis_;
  // variables used to calculate the math form
  Float distance_, sd_;
  // math form for this restraint (typically one of the harmonics)
  Score_Func* score_func_;
};

// Restrict particle position to interior of a torus
class IMPDLLEXPORT RSR_Torus : public Restraint
{
public:
  RSR_Torus(Model& model,
            Particle* p1,
            const Float main_radius,
            const Float tube_radius,
            const Float sd,
            Score_Func* score_func);
  virtual ~RSR_Torus();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  // variables used to determine the distance
  Float_Index x1_, y1_, z1_;
  // main radius of the torus
  Float main_radius_;
  // radius of the torus tube
  Float tube_radius_;
  // variables used to calculate the math form
  Float distance_, sd_;
  // math form for this restraint (typically one of the harmonics)
  Score_Func* score_func_;
};


// Restrict maximum distance between any two particles
class IMPDLLEXPORT RSR_Proximity : public Restraint
{
public:
  RSR_Proximity(Model& model,
                // couldn't get Swig to work with std::vector<Particle*>&
                std::vector<int>& particle_indexes,
                const Float mean,
                const Float sd,
                Score_Func* score_func);

  RSR_Proximity(Model& model,
                // couldn't get Swig to work with std::vector<Particle*>&
                std::vector<int>& particle_indexes,
                const std::string attr_name,
                const Float mean,
                const Float sd,
                Score_Func* score_func);

  virtual ~RSR_Proximity();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
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
  std::vector<RSR_Distance*> dist_rsrs_;
};

// Restrict max distance between at one or more pair of particles of any two sets of particles
class IMPDLLEXPORT RSR_Pair_Connectivity : public Restraint
{
public:
  RSR_Pair_Connectivity(Model& model,
                        // couldn't get Swig to work with std::vector<Particle*>&
                        std::vector<int>& particle1_indexes,
                        std::vector<int>& particle2_indexes,
                        const Float mean,
                        const Float sd,
                        Score_Func* score_func,
                        const int num_to_apply = 1,
                        const bool particle_reuse = false);

  RSR_Pair_Connectivity(Model& model,
                        // couldn't get Swig to work with std::vector<Particle*>&
                        std::vector<int>& particle1_indexes,
                        std::vector<int>& particle2_indexes,
                        const std::string attr_name,
                        const Float sd,
                        Score_Func* score_func,
                        const int num_to_apply,
                        const bool particle_reuse = false);

  virtual ~RSR_Pair_Connectivity();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  class Restraint_Score
  {
  public:
    Restraint_Score() {}
    ~Restraint_Score() {}
    void evaluate(void) {
      score_ = rsr_->evaluate(false);
    }
    bool operator<(const Restraint_Score& rs) {
      return score_ < rs.score_;
    }

    int part1_idx_;
    int part2_idx_;
    RSR_Distance* rsr_;
    Float score_;
  };

  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle1_indexes,
              std::vector<int>& particle2_indexes);

  // variables to determine the particle type
  std::vector<Int_Index> type_;

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
  std::list<Restraint_Score> rsr_scores_;
};


// Restrict max distance between at least one pair of particles of any two distinct types
class IMPDLLEXPORT RSR_Connectivity : public Restraint
{
public:
  RSR_Connectivity(Model& model,
                   // couldn't get Swig to work with std::vector<Particle*>&
                   std::vector<int>& particle_indexes,
                   const std::string type,
                   const Float mean,
                   const Float sd,
                   Score_Func* score_func);

  RSR_Connectivity(Model& model,
                   // couldn't get Swig to work with std::vector<Particle*>&
                   std::vector<int>& particle_indexes,
                   const std::string type,
                   const std::string attr_name,
                   const Float sd,
                   Score_Func* score_func);

  virtual ~RSR_Connectivity();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  // switch to using rsr_scores to allow STL sorting
  class Restraint_Score
  {
  public:
    Restraint_Score() {}
    ~Restraint_Score() {}
    void evaluate(void) {
      score_ = rsr_->evaluate(false);
    }
    bool operator<(const Restraint_Score& rs) {
      return score_ < rs.score_;
    }

    int part1_type_;
    int part2_type_;
    RSR_Distance* rsr_;
    Float score_;
  };

  /** restraints and their scores */
  std::list<Restraint_Score> rsr_scores_;

  void set_up(Model& model,
              // couldn't get Swig to work with std::vector<Particle*>&
              std::vector<int>& particle_indexes,
              const std::string type);

  // variables to determine the particle type
  std::vector<Int_Index> type_;

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
class IMPDLLEXPORT RSR_Exclusion_Volume : public Restraint
{
public:
  RSR_Exclusion_Volume(Model& model,
                       // couldn't get Swig to work with std::vector<Particle*>&
                       std::vector<int>& particle1_indexes,
                       std::vector<int>& particle2_indexes,
                       const std::string attr_name,
                       const Float sd
                      );

  RSR_Exclusion_Volume(Model& model,
                       // couldn't get Swig to work with std::vector<Particle*>&
                       std::vector<int>& particle_indexes,
                       const std::string attr_name,
                       const Float sd
                      );

  virtual ~RSR_Exclusion_Volume();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
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
  std::vector<RSR_Distance*> dist_rsrs_;
};

// Calculate score based on fit to EM map
class IMPDLLEXPORT RSR_EM_Coarse : public Restraint
{
public:
  RSR_EM_Coarse(Model& model,
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

  virtual ~RSR_EM_Coarse();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "0.0.1";
  }
  virtual std::string last_modified_by(void) {
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
  Float_Index *x_;
  Float_Index *y_;
  Float_Index *z_;

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

#endif
