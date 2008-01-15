/**
 *  \file VRMLLogOptimizerState.h   
 *  \brief A state which writes a series of VRML files.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_VRML_LOG_STATE_H
#define __IMP_VRML_LOG_STATE_H

#include <iostream>

#include "../../IMP_config.h"
#include "../../base_types.h"
#include "../../OptimizerState.h"

namespace IMP
{

//! A state that writes a series of VRML files.
/** The State writes a series of files generated from a 
    printf-style format string. These files contain spheres
    and points for a set of particles. The particles must have
    x,y,z coordinates (as defined by XYZDecorator) and,
    optionally have a radius.

    Documentation for the VRML file format can be found at
    http://www.cgl.ucsf.edu/chimera/docs/UsersGuide/bild.html
 */
class IMPDLLEXPORT VRMLLogOptimizerState : public OptimizerState
{
 public:
  VRMLLogOptimizerState(std::string filename,
                        const Particles &pis=Particles());
  virtual ~VRMLLogOptimizerState(){}

  virtual void update();

  virtual void show(std::ostream& out = std::cout) const;
  virtual std::string version() const {
    return "0.5";
  }
  virtual std::string last_modified_by() const {
    return "Daniel Russel";
  }
  //! Set the number of update calls to skip between writing files
  /** The first update call always writes a file.
   */
  void set_skip_steps(unsigned int i) {
    skip_steps_=i;
  }

  //! The float key to use for the radius
  /** Particles without such an attribute are drawn as fixed sized markers.
   */
  void set_radius(FloatKey k) {
    radius_=k;
  }
  //! The three color components
  /** Color values should be between 0 and 1. They will be snapped if needed.
   */
  void set_color(FloatKey r, FloatKey g, FloatKey b) {
    r_=r; g_=g; b_=b;
  }

  //! Set the particles to use.
  void set_particles(const Particles &pis) {
    pis_=pis;
  }
  void write(std::string name) const;

  //! A helper function to just write a list of particles to a file
  static void write(const Particles &pis,
                    FloatKey radius, FloatKey r,
                    FloatKey g, FloatKey b,
                    std::ostream &out);

protected:
  Particles pis_;
  std::string filename_;
  int file_number_;
  int call_number_;
  int skip_steps_;
  FloatKey radius_;
  FloatKey r_, g_, b_;
};


IMP_OUTPUT_OPERATOR(VRMLLogOptimizerState);

} // namespace IMP

#endif  /* __IMP_VRML_LOG_STATE_H */
