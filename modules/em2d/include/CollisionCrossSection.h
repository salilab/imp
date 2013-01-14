/**
 *  \file CollisionCrossSection.h
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_COLLISION_CROSS_SECTION_H
#define IMPEM2D_COLLISION_CROSS_SECTION_H



#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/project.h"
#include "IMP/macros.h"
#include "IMP/Particle.h"

IMPEM2D_BEGIN_NAMESPACE

class IMPEM2DEXPORT CollisionCrossSection: public IMP::base::Object {
public:

  /*!
    \param[in] n_projections Number of projections to use for obtaining the CCS
    \param[in] resolution "Resolution" to use to generate the projections.
    \param[in] pixel_size size in A/pix to use when generating the projections
    \param[in] projection_image_size. The size of the projections to generate
               for this value you should request a size so that:
               pixel_size * projection_image_size >= Max distance between two
               particles.
  */
  CollisionCrossSection(unsigned int  n_projections,
                        double resolution,
                        double pixel_size,
                        double projection_image_size
                        );


  /**
   * Particles used to calculate their collision cross section
   * @param ps The particles
   */
  void set_model_particles(const ParticlesTemp &ps);

  /**
   * Returns the collision cross section. Before using this function you need
   * to set the particles with set_model_particles()
   * @return The value of the collision cross section
   */
  double get_ccs() const;

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
  void show(std::ostream &out) const {
    out << "Collision Cross section." << std::endl;
  }
  IMP_OBJECT_METHODS( CollisionCrossSection);

protected:

  /*! Gets the sum of the values in the pixels with value > 0
    \param[in] m The projected area is computed for this matrix
    \param[out] Values of the area
  */
  double get_projected_area(const cv::Mat &m) const;

  unsigned int n_projections_;
  double resolution_;
  double pixel_size_;
  unsigned int img_size_;
  bool particles_set_;
  em2d::RegistrationResults regs_;
  cv::Mat average_projection_;
  em2d::MasksManagerPtr masks_manager_;
  double collision_cross_section_;
};
IMP_OBJECTS(CollisionCrossSection, CollisionCrossSections);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_COLLISION_CROSS_SECTION_H */
