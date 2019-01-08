/**
 *  \file tunneler_helpers.h
 *  \brief Helper functions for rigid body tunneler
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TUNNELER_HELPERS_H
#define IMPCORE_TUNNELER_HELPERS_H

#include <IMP/core/core_config.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <cmath>
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <Eigen/Geometry>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

  class Referential {
   private:
    Model* m_;
    ParticleIndex pi_;
    Eigen::Vector3d centroid_;
    Eigen::Matrix3d base_;
    Eigen::Quaterniond q_;
    Eigen::Vector3d t_;

   public:
    Referential() {}  // undefined behaviour, don't use
    Referential(Model* m, ParticleIndex pi);
    // return properties of this rigid body
    Eigen::Vector3d get_centroid() const { return centroid_; }
    Eigen::Matrix3d get_base() const { return base_; }
    Eigen::Quaterniond get_rotation() const { return q_; }
    // project some properties on this or the global frame's coordinates
    Eigen::Vector3d get_local_coords(const Eigen::Vector3d & other) const;
    Eigen::Quaterniond get_local_rotation(const Eigen::Quaterniond & other)
                                         const;

   private:
    Eigen::Vector3d compute_centroid() const;
    Eigen::Matrix3d compute_base() const;
    Eigen::Quaterniond compute_quaternion() const;
  };

  class Transformer {
   private:
    Model* m_;
    Referential ref_;
    ParticleIndex target_;
    Eigen::Vector3d t_;
    Eigen::Quaterniond q_;
    bool moved_;

   public:
    // reorient other by t and q, expressed in this reference frame
    Transformer(Model* m, const Referential& ref,
                ParticleIndex other, const Eigen::Vector3d& t,
                const Eigen::Quaterniond& q)
        : m_(m), ref_(ref), target_(other), t_(t), q_(q), moved_(true) {
      transform();
    }
    // no-op
    Transformer() : moved_(false) {}
    bool undo_transform();
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

   private:
    void transform();
  };

  struct Coord {
    typedef std::vector<Eigen::Vector3d> com_type;
    com_type coms;
    typedef std::vector<Eigen::Quaterniond,
            Eigen::aligned_allocator<Eigen::Quaterniond> > quats_type;
    quats_type quats;
    Coord() {}
    explicit Coord(Floats);
    Floats as_floats() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

//output of Coord, for debug purposes
std::ostream& operator<<(std::ostream& out, const Coord& c);

//helpers
Eigen::Quaterniond pick_positive(const Eigen::Quaterniond& q);

Coord get_coordinates_from_rbs(Model* m, ParticleIndexes pis,
                               ParticleIndex ref);
double get_squared_distance(const Coord& x, const Coord& y, double k);

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_TUNNELER_HELPERS_H */
