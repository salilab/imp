/**
 *  \file tunneler_helpers.h
 *  \brief Helper functions for rigid body tunneler
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TUNNELER_HELPERS_H
#define IMPCORE_TUNNELER_HELPERS_H

#include <IMP/core/core_config.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <cmath>
#include <IMP/algebra/eigen3/Eigen/StdVector>
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <IMP/algebra/eigen3/Eigen/Geometry>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

  class Referential {
   private:
    kernel::Model* m_;
    kernel::ParticleIndex pi_;
    IMP_Eigen::Vector3d centroid_;
    IMP_Eigen::Matrix3d base_;
    IMP_Eigen::Quaterniond q_;
    IMP_Eigen::Vector3d t_;

   public:
    Referential() {}  // undefined behaviour, don't use
    Referential(kernel::Model* m, kernel::ParticleIndex pi);
    // return properties of this rigid body
    IMP_Eigen::Vector3d get_centroid() const { return centroid_; }
    IMP_Eigen::Matrix3d get_base() const { return base_; }
    IMP_Eigen::Quaterniond get_rotation() const { return q_; }
    // project some properties on this or the global frame's coordinates
    IMP_Eigen::Vector3d get_local_coords(const IMP_Eigen::Vector3d
                                         & other) const;
    IMP_Eigen::Quaterniond get_local_rotation(const IMP_Eigen::Quaterniond
                                              & other) const;

   private:
    IMP_Eigen::Vector3d compute_centroid() const;
    IMP_Eigen::Matrix3d compute_base() const;
    IMP_Eigen::Quaterniond compute_quaternion() const;
  };

  class Transformer {
   private:
    kernel::Model* m_;
    Referential ref_;
    kernel::ParticleIndex target_;
    IMP_Eigen::Vector3d t_;
    IMP_Eigen::Quaterniond q_;
    bool moved_;

   public:
    // reorient other by t and q, expressed in this reference frame
    Transformer(kernel::Model* m, const Referential& ref,
                kernel::ParticleIndex other, const IMP_Eigen::Vector3d& t,
                const IMP_Eigen::Quaterniond& q)
        : m_(m), ref_(ref), target_(other), t_(t), q_(q), moved_(true) {
      transform();
    }
    // no-op
    Transformer() : moved_(false) {}
    bool undo_transform();
    IMP_EIGEN_MAKE_ALIGNED_OPERATOR_NEW

   private:
    void transform();
  };

  struct Coord {
    typedef std::vector<IMP_Eigen::Vector3d> com_type;
    com_type coms;
    typedef std::vector<IMP_Eigen::Quaterniond,
            IMP_Eigen::aligned_allocator<IMP_Eigen::Quaterniond> > quats_type;
    quats_type quats;
    Coord() {}
    explicit Coord(Floats);
    Floats as_floats() const;

    IMP_EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };

//output of Coord, for debug purposes
std::ostream& operator<<(std::ostream& out, const Coord& c);

//helpers
IMP_Eigen::Quaterniond pick_positive(const IMP_Eigen::Quaterniond& q);

Coord get_coordinates_from_rbs(kernel::Model* m, kernel::ParticleIndexes pis,
                               kernel::ParticleIndex ref);
double get_squared_distance(const Coord& x, const Coord& y, double k);

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_TUNNELER_HELPERS_H */
