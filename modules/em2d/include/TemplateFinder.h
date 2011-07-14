/**
 *  \file TemplateFinder
 *  \brief Find templates in images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_TEMPLATE_FINDER_H
#define IMPEM2D_TEMPLATE_FINDER_H

#include "IMP/em2d/Image.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/Particle.h"
#include "IMP/Object.h"
#include <algorithm>

IMPEM2D_BEGIN_NAMESPACE

//! Matching of the projections of a set of particles in an image
class IMPEM2DEXPORT TemplateFinder: public Object {
  IMP_OBJECT_INLINE(TemplateFinder, show(out), {});

protected:
  Pointer<Image> subject_;
  // Projections of the particles and the parameters used to generate them
  Images projections_;
  RegistrationResults projections_information_;

  ParticlesTemp my_particles_;
  bool particles_set_,projections_created_, matches_computed_,
        projection_parameters_set_;
  double pixel_size_,resolution_;

  RegistrationResults best_matches_; // see do_compute_best_matches()

public:

  /*! Constructor
    \param[in] ps Set of particles. Their projections are matched in the image
    \param[in] img Image that works as a subject for matching
  */
  TemplateFinder(const String &name):
                  Object(name),
                    particles_set_(false),
                    projections_created_(false),
                    matches_computed_(false),
                    projection_parameters_set_(false) {
    subject_ = new Image();
  };

  TemplateFinder(const ParticlesTemp &ps, Image *img);

  void show(std::ostream &out = std::cout) const {
    out << "TemplateFinder" << std::endl;
  }

  /*! Set an image as the subject for matching
    \param[in] the image
  */
  void set_image(Image *img);

  /*! Set the particles to project to get matches
    \param[in] ps particles
  */
  void set_particles(const ParticlesTemp &ps);

  /*! Project the particles. The projections are created evenly distributed
    \param[in] n number of projections
  */
  void create_particles_projections(unsigned int n);


  /*! Set the parameters required for projecting the particles
    \param[in] pixel_size desired for the projections
    \param[in] Resolution used generating the projections
  */
  void set_projection_parameters(double pixel_size, double resolution) {
    pixel_size_ = pixel_size;
    resolution_ = resolution;
    projection_parameters_set_ = true;
  }

  /*! Compute the best matches for the projections of the particles.
      The results are stored as orientations and translations.
      - An orientation is the rotation applied to the set of particles
        for the match.
      - A translation is the translation applied to the set of particles for
        the match. (in pixels)
    \param[in] n Number of best matches requested
  */
  void do_compute_best_matches(unsigned int n);

  /*! Returns the best matches computed with do_compute_best_matches()
    \param[in]
  */
  RegistrationResults get_best_matches() const;
};
IMP_OBJECTS(TemplateFinder, TemplateFinders);



IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_TEMPLATE_FINDER_H */
