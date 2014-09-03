/*!
 *  \file IMP/em2d/RegistrationResult.h
 *  \brief Registration results class
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_REGISTRATION_RESULT_H
#define IMPEM2D_REGISTRATION_RESULT_H

#include "IMP/em2d/internal/rotation_helper.h"
#include "IMP/em2d/em2d_config.h"
#include "IMP/em/ImageHeader.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/algebra/Transformation2D.h"
#include "IMP/base/Pointer.h"
#include <string>

IMPEM2D_BEGIN_NAMESPACE

//! Class to manage registration results
/*!
  \note  Stores the rotation and in-plane translation needed to register a
  model with an EM image. The values can come from a coarse registration, or
  from a fine registration (optimized with simplex).
  In the second case, the function get_is_optimized_result() will return true
  Contains:
  - Rotation in 3D of the model to register its projection with an image
  - The rotation is understood as ZYZ
  - shift shift to apply to the projection of the model (after rotation) to
    align it with the given image
*/
class IMPEM2DEXPORT RegistrationResult {
 public:
  RegistrationResult();

  RegistrationResult(double phi, double theta, double psi,
                     algebra::Vector2D shift, int projection_index = 0,
                     int image_index = 0, String name = "");

  RegistrationResult(algebra::Rotation3D R,
                     algebra::Vector2D shift = algebra::Vector2D(0., 0.),
                     int projection_index = 0, int image_index = 0,
                     String name = "");

  inline double get_phi() const { return phi_; }
  inline double get_theta() const { return theta_; }
  inline double get_psi() const { return psi_; }

  inline algebra::Vector2D get_shift() const { return shift_; }

  inline algebra::Vector3D get_shift_3d() const {
    return algebra::Vector3D(shift_[0], shift_[1], 0.0);
  }

  //! Projection that best matches the image after coarse registration
  inline int get_projection_index() const { return projection_index_; }

  inline void set_projection_index(int index) { projection_index_ = index; }

  //! Image that has been registered
  inline int get_image_index() const { return image_index_; }

  //! Index of the image that is registered
  inline void set_image_index(int index) { image_index_ = index; }

  //! Name of the object
  inline String get_name() const { return name_; }

  //! Cross correlation coefficient between the image and the projection of
  //! the model after registration
  inline double get_ccc() const { return ccc_; }

  inline void set_ccc(double ccc) {
    ccc_ = ccc;
    is_optimized_result_ = false;
  }

  //! Returns the score computed by the ScoreFunction comparing an image
  //! and a projection
  double get_score() const { return Score_; }

  void set_score(double Score) {
    Score_ = Score;
    is_optimized_result_ = true;
  }

  //! Rotation to apply to the model
  inline void set_rotation(double phi, double theta, double psi) {
    phi_ = phi;
    theta_ = theta;
    psi_ = psi;
    R_ = algebra::get_rotation_from_fixed_zyz(phi_, theta_, psi_);
  }

  //! Rotation to apply to the model
  void set_rotation(algebra::Rotation3D R);

  //! Shift to apply to the projection to register
  inline void set_shift(algebra::Vector2D shift) { shift_ = shift; }

  inline void set_name(String name) { name_ = name; }

  //! adds an in-plane transformation to the result stored
  //! The translation is understood as a shift (in pixels)
  void add_in_plane_transformation(algebra::Transformation2D t);

  IMP_SHOWABLE(RegistrationResult);

  //! Writes a parseable result
  void write(std::ostream &out = std::cout) const;

  //! Writes an info line to with the contents of a result line
  void write_comment_line(std::ostream &out = std::cout) const;

  //! read
  void read(const String &s);

  //! Gets a random result
  void set_random_registration(unsigned int index, double maximum_shift);

  //! Returns the rotation for the 3 projection angles
  inline algebra::Rotation3D get_rotation() const { return R_; }

  //! Sets the registration results to the header of an image
  void set_in_image(em::ImageHeader &header) const;

  //! Reads the registration parameters from an image
  void read_from_image(const em::ImageHeader &header);

  bool get_is_optimized_result() const { return is_optimized_result_; }

  ~RegistrationResult();

 protected:
  //! Translation in pixels and the rows first (y rows, x columns)
  algebra::Vector2D shift_;
  //! Cross correlation coefficient
  double ccc_;

  //! Score
  double Score_;
  //! name and index of the projection compared
  String name_;
  int projection_index_;
  //! index of the image being registered
  int image_index_;
  //! Euler angles (ZYZ)
  double phi_, theta_, psi_;
  algebra::Rotation3D R_;
  // false when the RegistrationResult is built from a rotation directly
  bool angles_defined_, is_optimized_result_;
};
IMP_VALUES(RegistrationResult, RegistrationResults);

//! Reads a set of registration results
IMPEM2DEXPORT RegistrationResults
    read_registration_results(const String &filename);

//! Writes a set of registration results
IMPEM2DEXPORT void write_registration_results(
    String filename, const RegistrationResults &results);

//! Provides a set of random registration results (or parameters)
/*!
  \param[in] n the number of requested results
  \param[in] maximum_shift shift from the center in pixels
*/
IMPEM2DEXPORT RegistrationResults
    get_random_registration_results(unsigned int n, double maximum_shift = 5.0);

//! Provides a set of registration results with directions of projection
//!      evenly distributed in the hemisphere
/*!
  \param[in] n_projections the number of requested projections
*/
IMPEM2DEXPORT RegistrationResults
    get_evenly_distributed_registration_results(unsigned int n_projections);

inline double get_random_between_zero_and_one() {
  return (static_cast<double>(rand()) / (static_cast<double>(RAND_MAX) + 1));
}

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_REGISTRATION_RESULT_H */
