/**
 *  \file CoarseEMRestraint.h    Calculate score based on fit to EM map.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_COARSE_EM_RESTRAINT_H
#define __IMP_COARSE_EM_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../emfile.h"
#include "Restraint.h"

namespace IMP
{

// Calculate score based on fit to EM map
class IMPDLLEXPORT CoarseEMRestraint : public Restraint
{
public:
  CoarseEMRestraint(Model& model,
                std::vector<int>& particle_indexes,
                EM_Density *emdens,
                int nx,
                int ny,
                int nz,
                float pixelsize,
                float resolution,
                std::string radius_str,
                std::string weight_str,
                EM_Gridcoord *gridcd,
                float scalefac
               );

  virtual ~CoarseEMRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.0.1";
  }
  virtual std::string last_modified_by(void) const {
    return "Frido and Keren";
  }

protected:
  EM_Density *emdens_;
  int nx_;
  int ny_;
  int nz_;
  float pixelsize_;
  float resolution_;
  EM_Gridcoord *gridcd_;
  float scalefac_;

  int num_particles_;

  // weight and radius associated with each particle
  Float *radius_;
  Float *weight_;

  // coordinates to pass to EM score C routine
  FloatIndex *x_;
  FloatIndex *y_;
  FloatIndex *z_;

  // coordinates to pass to EM score C routine
  Float *cdx_;
  Float *cdy_;
  Float *cdz_;

  // partial derivs to pass to EM score C routine
  Float *dvx_;
  Float *dvy_;
  Float *dvz_;
};

} // namespace IMP

#endif /* __IMP_COARSE_EM_RESTRAINT_H */
