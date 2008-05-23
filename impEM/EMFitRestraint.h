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
                 DensityMap &em_map,
                 std::string radius_str,
                 std::string weight_str,
                 float scale);

  virtual ~EMFitRestraint();

  //! \return the predicted density map of the model
  SampledDensityMap *get_model_dens_map() {
    return model_dens_map_;
  }

  IMP_RESTRAINT(VersionInfo("Keren", "0.0.1"))

private:

  DensityMap *target_dens_map_;
  SampledDensityMap *model_dens_map_;
  // reference to the IMP environment
  float scalefac_;
  int num_particles_;
  IMPParticlesAccessPoint access_p_;
  // derivatives
  vector<Float> dx_, dy_ ,dz_;
};



} //  namespace IMP


#endif /* _EMFITRESTRAINT_H */
