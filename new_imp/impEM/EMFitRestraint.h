#ifndef _EMFITRESTRAINT_H
#define _EMFITRESTRAINT_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Bret
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT

*/



#include "IMP_config.h"
#include "Model.h"

#include "DensityMap.h"
#include "CoarseCC.h"
#include "SampledDensityMap.h"
#include "IMPParticlesAccessPoint.h"

namespace IMP
{

//! Calculate score based on fit to EM map
class  IMPDLLEXPORT EMFitRestraint : public Restraint
{
public:

  EMFitRestraint(Model& model,
                 std::vector<int>& particle_indexes, //the selection
                 DensityMap &em_map_,
                 std::string radius_str,
                 std::string weight_str,
                 float scale_);

  virtual ~EMFitRestraint();

  virtual Float evaluate(bool calc_deriv);

  //! status
  virtual void show (std::ostream& out = std::cout) {
    // TODO - add implementation
  }
  virtual std::string version(void) {
    return "0.0.1";
  }
  virtual std::string last_modified_by(void) {
    return "Frido and Keren";
  }

private:

  DensityMap target_dens_map;
  SampledDensityMap model_dens_map;
  //! reference to the IMP environment
  Model model;
  float scalefac;
  int num_particles;
  IMPParticlesAccessPoint access_p;
  //! derivaties
  vector<Float> dx,dy,dz;
};



} //  namespace IMP


#endif /* _EMFITRESTRAINT_H */
