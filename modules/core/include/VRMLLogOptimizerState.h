/**
 *  \file VRMLLogOptimizerState.h
 *  \brief A state which writes a series of VRML files.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_VRML_LOG_OPTIMIZER_STATE_H
#define IMPCORE_VRML_LOG_OPTIMIZER_STATE_H

#include "core_exports.h"
#include "internal/core_version_info.h"
#include "SingletonContainer.h"

#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>
#include <IMP/ParticleRefiner.h>
#include <IMP/Vector3D.h>

#include <iostream>
#include <map>
#include <boost/tuple/tuple.hpp>

IMPCORE_BEGIN_NAMESPACE

//! A state that writes a series of VRML files.
/** The State writes a series of files generated from a
    printf-style format string. These files contain spheres
    and points for a set of particles. The particles must have
    x,y,z coordinates (as defined by XYZDecorator) and,
    optionally have a radius.

    Documentation for the VRML file format can be found at
    http://www.cgl.ucsf.edu/chimera/docs/UsersGuide/bild.html

    \ingroup log
 */
class IMPCOREEXPORT VRMLLogOptimizerState : public OptimizerState
{
 public:
  VRMLLogOptimizerState(SingletonContainer *pc,
                        std::string filename);
  virtual ~VRMLLogOptimizerState(){}

  IMP_OPTIMIZER_STATE(internal::core_version_info)

  //! Set the number of update calls to skip between writing files
  /** The first update call always writes a file.
   */
  void set_skip_steps(unsigned int i) {
    skip_steps_=i;
  }

  //! The float key to use for the radius
  /** Particles without such an attribute are drawn as fixed sized markers.
   */
  void set_radius_key(FloatKey k) {
    radius_=k;
  }

  //! The integer key to use for the color.
  void set_color_key(IntKey k) {
    color_=k;
  }

  //! Set the RGB color to use for a given particle color value.
  /** \param[in] c  Value of the particle's color IntKey.
      \param[in] v  Color as an RGB vector. Color values should be between 0
                    and 1. They will be snapped if needed.
   */
  void set_color(int c, Vector3D v);

  void set_singleton_container(SingletonContainer *pc) {
    pc_=pc;
  }

  IMP_CONTAINER(ParticleRefiner, particle_refiner, ParticleRefinerIndex);

  //! Force it to write the next file
  void write_next_file();

  /** Write to a file with the given name */
  void write(std::string name) const;

private:
  //! A helper function to just write a list of particles to a file
  template <class It>
  void write(std::ostream &out, It b, It e) const;
  std::string filename_;
  int file_number_;
  int call_number_;
  int skip_steps_;
  FloatKey radius_;
  IntKey color_;
  std::map<int, Vector3D > colors_;
  Pointer<SingletonContainer> pc_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_VRML_LOG_OPTIMIZER_STATE_H */
