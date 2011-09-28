/**
 *  \file CollisionCrossSection.h
 *  \brief
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_COLLISION_CROSS_SECTION_H
#define IMPEM2D_COLLISION_CROSS_SECTION_H



#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/Image.h"
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


  /*! Feed the particles to the class so the CCS can be computed
    \param[in] ps
  */
  void set_model_particles(const ParticlesTemp &ps);

  /*! Returns the CCS of the particles. Before using this function the
    particles need to be set with set_model_particles()
  */
  double get_ccs() const;

  void show(std::ostream &out) const {
    out << "Collision Cross section." << std::endl;
  }

  IMP_OBJECT_INLINE( CollisionCrossSection, show(out), {} );

protected:

  // This function gets a matrix and computes the "projected area". That is,
  // The sum of the areas of all the pixels with value > 0
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
IMP_OUTPUT_OPERATOR(CollisionCrossSection);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_COLLISION_CROSS_SECTION_H */
