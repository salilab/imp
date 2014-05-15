/**
 *  \file IMP/em2d/internal/Projection.cpp
 *  \brief A class for generation and storage of projections
 *
 *  \authors Dina Schneidman
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em2d/internal/Projection.h>

#include <IMP/em/KernelParameters.h>
#include <IMP/algebra/SphericalVector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/atom/Mass.h>
#include <IMP/constants.h>
#include <boost/scoped_ptr.hpp>

#include <algorithm>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

std::map<double, std::vector<Projection::MaskCell> > Projection::radius2mask_;

Projection::Projection(const IMP::algebra::Vector3Ds& points,
                       const std::vector<double>& radii,
                       const std::vector<double>& mass,
                       double scale, double resolution, int axis_size)
  : scale_(scale), t_i_(0), t_j_(0), resolution_(resolution) {

  double max_radius = *std::max_element(radii.begin(), radii.end());
  init(points, max_radius, axis_size);
  project(points, radii, mass);
}

Projection::Projection(const IMP::algebra::Vector3Ds& points,
                       const IMP::algebra::Vector3Ds& ligand_points,
                       const std::vector<double>& ligand_radii,
                       const std::vector<double>& ligand_mass,
                       double scale, double resolution, int axis_size)
    : scale_(scale), t_i_(0), t_j_(0), resolution_(resolution) {

  double max_radius = *std::max_element(ligand_radii.begin(),
                                        ligand_radii.end());
  init(points, max_radius, axis_size);
  project(ligand_points, ligand_radii, ligand_mass);
}

void Projection::init(const IMP::algebra::Vector3Ds& points,
                      double max_radius, int axis_size) {
  // determine the image size needed to store the projection
  x_min_ = y_min_ = std::numeric_limits<float>::max();
  x_max_ = y_max_ = std::numeric_limits<float>::min();

  // determine translation to center
  algebra::Vector3D center(0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < points.size(); i++) {
    x_min_ = std::min(x_min_, points[i][0]);
    y_min_ = std::min(y_min_, points[i][1]);
    x_max_ = std::max(x_max_, points[i][0]);
    y_max_ = std::max(y_max_, points[i][1]);
    center += points[i];
  }
  center /= points.size();

  static IMP::em::KernelParameters kp(resolution_);
  const IMP::em::RadiusDependentKernelParameters& params =
      kp.get_params(max_radius);

  double wrap_length = 2 * params.get_kdist() + 1.0;
  x_min_ = x_min_ - wrap_length - scale_;
  y_min_ = y_min_ - wrap_length - scale_;
  x_max_ = x_max_ + wrap_length + scale_;
  y_max_ = y_max_ + wrap_length + scale_;

  int width = (int)((x_max_ - x_min_) / scale_ + 2);
  int height = (int)((y_max_ - y_min_) / scale_ + 2);
  int size = std::max(width, height);

  // the determined size should be below axis size if specified
  if (axis_size > 0 && size <= axis_size)
    size = axis_size;
  else {
    std::cerr << "wrong size estimate " << size << " vs. estimate " << axis_size
              << std::endl;
  }
  this->resize(boost::extents[size][size]);

  int i = 0, j = 0;
  get_index_for_point(center, i, j);
  t_i_ = size / 2 - i;
  t_j_ = size / 2 - j;
}

void Projection::project(const IMP::algebra::Vector3Ds& points,
                         const std::vector<double>& radii,
                         const std::vector<double>& mass) {
  int i, j;
  for (unsigned int p_index = 0; p_index < points.size(); p_index++) {
    // get mask
    const std::vector<MaskCell>& mask = get_sphere_mask(radii[p_index]);
    // project
    if (get_index_for_point(points[p_index], i, j)) {
      for (unsigned int mask_index = 0; mask_index < mask.size();
           mask_index++) {
        int i_shift = mask[mask_index].i;
        int j_shift = mask[mask_index].j;
        double density = mask[mask_index].d;
        (*this)[i + i_shift][j + j_shift] += mass[i] * density;
      }
    }
  }
}

const std::vector<Projection::MaskCell>& Projection::get_sphere_mask(double radius) {
  // check if already calculated
  if(radius2mask_.find(radius) == radius2mask_.end()) {
    // compute
    std::vector<MaskCell> mask;
    calculate_sphere_mask(mask, radius);
    radius2mask_[radius] = mask;
  }
  return radius2mask_.find(radius)->second;
}

void Projection::calculate_sphere_mask(std::vector<MaskCell>& mask,
                                       double radius) {
  static IMP::em::KernelParameters kp(resolution_);
  const IMP::em::RadiusDependentKernelParameters& params =
      kp.get_params(radius);

  int int_radius = symm_round(params.get_kdist() / scale_) + 1;
  int int_radius2 = IMP::base::square(int_radius);

  double normalization = -scale_ * scale_ * params.get_inv_sigsq();
  double normalization2 = params.get_normfac();

  int i_bound, j_bound, k_bound;
  i_bound = int_radius;
  // iterate circle indices
  for (int i = -i_bound; i <= i_bound; i++) {
    j_bound = (int)sqrt(static_cast<double>(int_radius2 - i * i));
    for (int j = -j_bound; j <= j_bound; j++) {
      k_bound = (int)sqrt(static_cast<double>(int_radius2 - i * i - j * j));
      MaskCell ms(i, j, 0.0);
      for (int k = -k_bound; k <= k_bound; k++) {
        int int_dist2 = i * i + j * j + k * k;
        double density = exp(normalization * int_dist2);
        if (density > 0.1) {
          density *= normalization2;
          ms.d += density;
        }
      }
      mask.push_back(ms);
    }
  }
}

void Projection::add(const Projection& p) {
  // calculate translation
  int t_i = t_i_ - p.t_i_ - symm_round((y_min_ - p.y_min_) / scale_);
  int t_j = t_j_ - p.t_j_ - symm_round((x_min_ - p.x_min_) / scale_);

  for (int i = 0; i < get_height(); i++) {
    for (int j = 0; j < get_width(); j++) {
      if (p[i][j] > 0 && i + t_i > 0 && j + t_j > 0 && i + t_i < get_height() &&
          j + t_j < get_width())
        (*this)[i + t_i][j + t_j] += p[i][j];
    }
  }
}

double compute_max_distance(const IMP::algebra::Vector3Ds& coordinates) {
  double max_dist2 = 0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      double dist2 =
          IMP::algebra::get_squared_distance(coordinates[i], coordinates[j]);
      if (dist2 > max_dist2) max_dist2 = dist2;
    }
  }
  return sqrt(max_dist2);
}

namespace {
void quasi_evenly_spherical_distribution(unsigned long N,
                                         IMP::algebra::SphericalVector3Ds& vs,
                                         double r) {
  vs.resize(N);
  double theta, psi;
  for (unsigned long k = 1; k <= N; ++k) {
    double h = (2.0 * (k - 1)) / (N - 1) - 1.0;
    theta = std::acos(h);
    if (k == 1 || k == N) {
      psi = 0;
    } else {
      psi = (vs[k - 2][2] + 3.6 / sqrt((double)N * (1.0 - h * h)));
      int div = psi / (2 * IMP::PI);
      psi -= div * 2 * IMP::PI;
    }
    // Set the values of the spherical vector
    vs[k - 1][0] = r;
    vs[k - 1][1] = theta;
    vs[k - 1][2] = psi;
  }
}
}

void compute_projections(const kernel::Particles& particles,
                         unsigned int projection_number, double pixel_size,
                         double resolution,
                         boost::ptr_vector<Projection>& projections,
                         int image_size) {

  // get coordinates, radius and mass
  IMP::algebra::Vector3Ds points(particles.size());
  std::vector<double> radii(particles.size());
  std::vector<double> mass(particles.size());
  for (unsigned int i = 0; i < particles.size(); i++) {
    points[i] = core::XYZ(particles[i]).get_coordinates();
    radii[i] = core::XYZR(particles[i]).get_radius();
    mass[i] = atom::Mass(particles[i]).get_mass();
  }

  // estimate max image size
  double max_dist = compute_max_distance(points);
  double max_radius = *std::max_element(radii.begin(), radii.end());
  static IMP::em::KernelParameters kp(resolution);
  const IMP::em::RadiusDependentKernelParameters& params =
    kp.get_params(max_radius);
  double wrap_length = 2 * params.get_kdist() + 1.0;
  int axis_size =
      (int)((max_dist + 2 * wrap_length + 2 * pixel_size) / pixel_size + 2);
  if (axis_size <= image_size) axis_size = image_size;

  // storage for rotated points
  IMP::algebra::Vector3Ds rotated_points(points.size());
  // points on sphere
  IMP::algebra::SphericalVector3Ds spherical_coords;
  quasi_evenly_spherical_distribution(projection_number, spherical_coords, 1.0);

  for (unsigned int i = 0; i < spherical_coords.size(); i++) {
    // convert sphere coordinate to rotation
    IMP::algebra::SphericalVector3D v = spherical_coords[i];
    double cy = cos(v[1] / 2.0);
    double cz = cos(v[2] / 2.0);
    double sy = sin(v[1] / 2.0);
    double sz = sin(v[2] / 2.0);
    // this is a rotation about z axis by an angle v[2]
    // followed by rotation about y axis by an angle v[1]
    IMP::algebra::Rotation3D r(cy * cz, sy * sz, sy * cz, cy * sz);

    // rotate points
    for (unsigned int point_index = 0; point_index < points.size();
         point_index++) {
      rotated_points[point_index] = r * points[point_index];
    }
    // project
    std::auto_ptr<Projection> p(new Projection(rotated_points, radii, mass,
                                               pixel_size, resolution,
                                               axis_size));
    p->set_rotation(r);
    p->set_axis(IMP::algebra::Vector3D(v.get_cartesian_coordinates()));
    p->set_id(i);
    // rasmol
    // std::cout << "#projection " << i+1
    //<<"\nreset\nrotate Z " << RAD_2_DEG(v[2]);
    // std::cout << "\nrotate Y -" << IMP_RAD_2_DEG(v[1]) << std::endl;
    // chimera
    // std::cout << "#projection " << i+1
    //<<"\nreset;turn x 180; turn z -" <<  IMP_RAD_2_DEG(v[2]);
    // std::cout << ";turn y -" << IMP_RAD_2_DEG(v[1])
    // << ";wait;sleep 3;"<< std::endl;
    // string file_name = "projection_" +
    //   string(boost::lexical_cast<string>(i+1)) + ".pgm";
    // p.write_PGM(file_name);
    projections.push_back(p.release());
  }
}

void compute_projections(const kernel::Particles& all_particles,
                         const kernel::Particles& lig_particles,
                         unsigned int projection_number,
                         double pixel_size, double resolution,
                         boost::ptr_vector<Projection>& projections,
                         int image_size) {

  // get coordinates, radius and mass
  IMP::algebra::Vector3Ds all_points(all_particles.size());
  std::vector<double> all_radii(all_particles.size());
  for (unsigned int i = 0; i < all_particles.size(); i++) {
    all_points[i] = core::XYZ(all_particles[i]).get_coordinates();
    all_radii[i] = core::XYZR(all_particles[i]).get_radius();
  }

  IMP::algebra::Vector3Ds lig_points(lig_particles.size());
  std::vector<double> lig_radii(lig_particles.size());
  std::vector<double> lig_mass(lig_particles.size());
  for (unsigned int i = 0; i < lig_particles.size(); i++) {
    lig_points[i] = core::XYZ(lig_particles[i]).get_coordinates();
    lig_radii[i] = core::XYZR(lig_particles[i]).get_radius();
    lig_mass[i] = atom::Mass(lig_particles[i]).get_mass();
  }

  int axis_size = image_size;
  // use image_size if given
  if (!(image_size > 0)) {
    double max_dist = compute_max_distance(all_points);
    static IMP::em::KernelParameters kp(resolution);
    double radius = 3.0;
    const IMP::em::RadiusDependentKernelParameters& params =
        kp.get_params(radius);
    double wrap_length = 2 * params.get_kdist() + 1.0;
    axis_size =
        (int)((max_dist + 2 * wrap_length + 2 * pixel_size) / pixel_size + 2);
    if (axis_size <= image_size) axis_size = image_size;
  }

  // storage for rotated points
  IMP::algebra::Vector3Ds rotated_points(all_points.size());
  IMP::algebra::Vector3Ds rotated_ligand_points(lig_points.size());
  // points on a sphere
  IMP::algebra::SphericalVector3Ds spherical_coords;
  quasi_evenly_spherical_distribution(projection_number, spherical_coords, 1.0);

  for (unsigned int i = 0; i < spherical_coords.size(); i++) {
    // convert sphere coordinate to rotation
    IMP::algebra::SphericalVector3D v = spherical_coords[i];
    double cy = cos(v[1] / 2.0);
    double cz = cos(v[2] / 2.0);
    double sy = sin(v[1] / 2.0);
    double sz = sin(v[2] / 2.0);
    // this is a rotation about z axis by an angle v[2]
    // followed by rotation about y axis by an angle v[1]
    IMP::algebra::Rotation3D r(cy * cz, sy * sz, sy * cz, cy * sz);

    // rotate points
    for (unsigned int p_index = 0; p_index < all_points.size(); p_index++)
      rotated_points[p_index] = r * all_points[p_index];
    for (unsigned int p_index = 0; p_index < lig_points.size(); p_index++)
      rotated_ligand_points[p_index] = r * lig_points[p_index];

    // project
    std::auto_ptr<Projection> p(
        new Projection(rotated_points, rotated_ligand_points, lig_radii,
                       lig_mass, pixel_size, resolution, axis_size));
    p->set_rotation(r);
    p->set_axis(IMP::algebra::Vector3D(v.get_cartesian_coordinates()));
    p->set_id(i);
    // rasmol
    // std::cout << "#projection " << i+1 <<"\nreset\nrotate Z "
    //<< IMP_RAD_2_DEG(v[2]);
    // std::cout << "\nrotate Y -" << IMP_RAD_2_DEG(v[1]) << std::endl;
    // chimera
    // std::cout << "#projection " << i+1
    //<<"\nreset;turn x 180; turn z -" << IMP_RAD_2_DEG(v[2]);
    // std::cout << ";turn y -" << IMP_RAD_2_DEG(v[1])
    //<< ";wait;sleep 3;"<< std::endl;
    // string file_name = "projection_" +
    //   string(boost::lexical_cast<string>(i+1)) + ".pgm";
    // p.write_PGM(file_name);
    projections.push_back(p.release());
  }
}

IMPEM2D_END_INTERNAL_NAMESPACE
