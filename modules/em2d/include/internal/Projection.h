/**
 * \file Projection \brief basic projection class
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_INTERNAL_PROJECTION_H
#define IMPEM2D_INTERNAL_PROJECTION_H

#include <IMP/em2d/em2d_config.h>

#include "Image2D.h"

#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/Particle.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZR.h>


#include <boost/ptr_container/ptr_vector.hpp>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

// Information about a projection
struct ProjectionInfo {
  algebra::Rotation3D rotation; // rotation to make projection from Particles
  algebra::Vector3D centroid; // coordinates of the centroid in model space
};

class IMPEM2DEXPORT Projection : public Image2D<> {
 public:

  //! Constructor
  /**
     \param[in] particles Particles to project, should have XYZ and mass
     \param[in] scale Image scale - pixel size
     \param[in] resolution Estimated resolution of the images
  */
  Projection(const IMP::algebra::Vector3Ds& points,
             const std::vector<double>& mass,
             double scale, double resolution, int axis_size = 0);

  //! Constructor
  /**
     \param[in] particles All particles in the image
     \param[in] ligand_particles Ligand only particles
     \param[in] scale Image scale - pixel size
     \param[in] resolution Estimated resolution of the images
  */
  Projection(const IMP::algebra::Vector3Ds& points,
             const IMP::algebra::Vector3Ds& ligand_points,
             const std::vector<double>& ligand_mass,
             double scale, double resolution, int axis_size = 0);

  IMP::algebra::Vector3D get_axis() const { return axis_; }
  IMP::algebra::Rotation3D get_rotation() const { return rotation_; }
  double get_scale() const { return scale_; }
  int get_id() const { return id_; }

  void set_rotation(IMP::algebra::Rotation3D& r) { rotation_ = r; }
  void set_axis(IMP::algebra::Vector3D v) { axis_ = v; }
  void set_id(int id) { id_ = id; }

  void add(const Projection& p);

  algebra::Vector3D get_point_for_index(double x, double y) const {
    algebra::Vector3D point;
    point[0] = (x - t_j_) * scale_ + x_min_;
    point[1] = (y - t_i_) * scale_ + y_min_;
    point[2] = 0.;
    return point;
  }

 private:
  // Make noncopyable
  Projection(Projection&) : Image2D<>() {};

  class MaskCell {
  public:
    MaskCell(int ii, int jj, double dd) : i(ii), j(jj), d(dd) {}
    int i, j;
    double d;  // density
  };

  void init(const IMP::algebra::Vector3Ds& points, int axis_size);

  void project(const IMP::algebra::Vector3Ds& points,
               const std::vector<double>& mass);

  const std::vector<MaskCell>& get_sphere_mask();

  void calculate_sphere_mask(std::vector<MaskCell>& mask);

  bool get_index_for_point(const IMP::algebra::Vector3D& point, int& i,
                           int& j) const {
    if (x_min_ > point[0] || y_min_ > point[1] || x_max_ < point[0] ||
        y_max_ < point[1])
      return false;
    j = symm_round((point[0] - x_min_) / scale_) + t_j_;
    i = symm_round((point[1] - y_min_) / scale_) + t_i_;
    return true;
  }

  // gaussian mask
  static std::vector<MaskCell> mask_;

 private:
  int id_;
  double scale_;
  double x_min_, y_min_, x_max_, y_max_;
  int t_i_, t_j_;
  double resolution_;
  IMP::algebra::Rotation3D rotation_;
  IMP::algebra::Vector3D axis_;
};

IMPEM2DEXPORT
double compute_max_distance(const IMP::algebra::Vector3Ds& points);

IMPEM2DEXPORT
void compute_projections(const IMP::Particles& particles,
                         unsigned int projection_number, double pixel_size,
                         double resolution,
                         boost::ptr_vector<Projection>& projections,
                         int image_size = 0);

IMPEM2DEXPORT
void compute_projections(const Particles& all_particles,
                         const Particles& lig_particles,
                         unsigned int projection_number, double pixel_size,
                         double resolution,
                         boost::ptr_vector<Projection>& projections,
                         int image_size = 0);

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_PROJECTION_H */
