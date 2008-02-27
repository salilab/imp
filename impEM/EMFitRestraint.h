#ifndef _EMFITRESTRAINT_H
#define _EMFITRESTRAINT_H


#include "IMP/Model.h"
#include "IMP/Restraint.h"
#include "IMP/VersionInfo.h"
#include "IMPEM_config.h"
#include "DensityMap.h"
#include "CoarseCC.h"
#include "SampledDensityMap.h"
#include "IMPParticlesAccessPoint.h"

namespace IMP
{

//! Calculate score based on fit to EM map
class IMPEMDLLEXPORT EMFitRestraint : public Restraint
{
public:

  EMFitRestraint(Model& model,
                 std::vector<int>& particle_indexes, //the selection
                 DensityMap &em_map_,
                 std::string radius_str,
                 std::string weight_str,
                 float scale_);

  virtual ~EMFitRestraint();

  IMP_RESTRAINT(VersionInfo("Keren", "0.0.1"))

private:

  DensityMap *target_dens_map;
  SampledDensityMap *model_dens_map;
  // reference to the IMP environment
  float scalefac;
  int num_particles;
  IMPParticlesAccessPoint access_p;
  // derivatives
  vector<Float> dx,dy,dz;
};



} //  namespace IMP


#endif /* _EMFITRESTRAINT_H */
