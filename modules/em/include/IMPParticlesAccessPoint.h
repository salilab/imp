/**
 *  \file IMPParticlesAccessPoint.h
 *  \brief Provision of EMBED structural data using the IMP framework.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_IMP_PARTICLES_ACCESS_POINT_H
#define IMPEM_IMP_PARTICLES_ACCESS_POINT_H

#include "config.h"
#include "ParticlesAccessPoint.h"
#include <IMP/core/XYZDecorator.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

#include <vector>
#include <map>

IMPEM_BEGIN_NAMESPACE

#define X_ATT_NAME "x"
#define Y_ATT_NAME "y"
#define Z_ATT_NAME "z"

//!
class IMPEMEXPORT IMPParticlesAccessPoint : public ParticlesAccessPoint
{

public:
  IMPParticlesAccessPoint() {}
  //! Constructor
  /** \param[in] particles the particles
      \param[in] radius_key the attribute name of the radius
      \param[in] weight_key the attribute name of the weight
   */
  IMPParticlesAccessPoint(const Particles &particles,
                          FloatKey radius_key,
                          FloatKey weight_key);

  void reselect(const Particles& ps);
  //! Get the number of particles
  int get_size() const {
    return particles_.size();
  }
  //! Get the value of the x coordinate of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \return the value of the x coordinate of particle particles_[ind]
   */
  float get_x(unsigned int ind) const {
    return particles_[ind]->get_value(get_x_key());
  }
  //! Get the value of the y coordinate of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \return the value of the y coordinate of particle particles_[ind]
   */
  float get_y(unsigned int ind) const {
    return particles_[ind]->get_value(get_y_key());
  }
  //! Get the value of the z coordinate of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \return the value of the z coordinate of particle particles_[ind]
   */
  float get_z(unsigned int ind) const {
    return particles_[ind]->get_value(get_z_key());
  }
  //! Get the value of the radius attribute of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \return the value of the radius attribute of particle particles_[ind]
   */
  float get_r(unsigned int ind) const {
    std::cout<<"size ::: " << particles_.size() << std::endl;
    std::cout<<"ind ::: " << ind << std::endl;
    particles_[ind]->show();
    std::cout << radius_key_ <<std::endl;
    std::cout<<"val ::: " << particles_[ind]->get_value(radius_key_)
             << std::endl;
    std::cout << "after " << std::endl;
    return particles_[ind]->get_value(radius_key_);
  }
  //! Get the value of the weight attribute of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \return the value of the weight attribute of particle particles_[ind]
   */
  float get_w(unsigned int ind) const {
    return particles_[ind]->get_value(weight_key_);
  }
  //! Set the value of the x coordinate of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \param[in] xval the value of the x coordinate
   */
  void set_x(unsigned int ind, float xval) {
    particles_[ind]->set_value(get_x_key(), xval);
  }
  //! Set the value of the y coordinate of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \param[in] yval the value of the y coordinate
   */
  void set_y(unsigned int ind, float yval) {
    particles_[ind]->set_value(get_y_key(), yval);
  }
  //! Set the value of the x coordinate of a particle
  /** \param[in] ind the position of a particle in the stored
                     vector of particles
      \param[in] zval the value of the z coordinate
   */
  void set_z(unsigned int ind, float zval) {
    particles_[ind]->set_value(get_z_key(), zval);
  }

  FloatKey get_x_key() const {
    return IMP::core::XYZDecorator::get_xyz_keys()[0];
  }
  FloatKey get_y_key() const {
    return IMP::core::XYZDecorator::get_xyz_keys()[1];
  }
  FloatKey get_z_key() const {
    return IMP::core::XYZDecorator::get_xyz_keys()[2];
  }

private:
  Particles particles_;
  FloatKey radius_key_, weight_key_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_IMP_PARTICLES_ACCESS_POINT_H */
