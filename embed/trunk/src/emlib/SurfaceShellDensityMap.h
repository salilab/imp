#ifndef _SURFACE_SHELL_DENSITY_MAP_H
#define _SURFACE_SHELL_DENSITY_MAP_H

#include <vector>
#include "EM_config.h"
#include "exp.h"
#include "SampledDensityMap.h"
#include "DensityHeader.h"
#include "def.h"
#include "ParticlesAccessPoint.h"

#define _DEFAULT_NUM_SHELLS 5
#define _RESOLUTION 3.0
#define _SIG_CUTOFF 3.0
#define _BACKGROUND_VAL 0.0
#define _SURFACE_VAL 1.0
//! The class repersents a molecule as shells of distance from the surface
/**
 */

class EMDLLEXPORT SurfaceShellDensityMap: public SampledDensityMap
{

public:

  //! Creates a new density map.
  /** The header of the map is not determined and no data is being allocated
   */
  SurfaceShellDensityMap();

  //! Creates a new density map.
  //! The size of the map is determined by the header and the data is allocated.
  SurfaceShellDensityMap(const DensityHeader &header);

  //! Generatea a surface shell density map from the input particles.
  /** /param[in] access_p     access point to the particles (locations,
                              radius, weight)
      /param[in] voxel_size   the voxel size.
      /note the voxel size and the number of shells determines
            the resolution/accuracy of the surface rasterization.
      /param[in] sig_cutoff   Choose what should be the sigma cutoff for
                 accurate sampling. It is used in two functions;
                 (i)  to determine the size of the grid dimensions
                 (ii) to determine the voxels around the coords participating
                      in the sampling procedure.
   */
  SurfaceShellDensityMap(const ParticlesAccessPoint &access_p,
                         float voxel_size,
                         int num_shells=_DEFAULT_NUM_SHELLS
                         );

  //! Resample the grid to consist of density shells of a model
  /**
  All voxels that are outside of the model defined by the particles will be
  set to zero.
  Voxels on the surface between the model and the background
  will be set of 1.
  The interior voxels will be assign value according to their
  corresponding shell ( the value increases as the voxel is farthrer away
  from the surface).
  */
void resample(const ParticlesAccessPoint &access_p);
protected:
  //! Set the value of the map voxels as either scene or background
  /**
    /param[in] access_p       the particles of the model (scene)
    /param[in] scene_val      all voxels corredponsing to particles will
                              be set to this value
   */
  void binaries(const ParticlesAccessPoint &access_p,float scene_val);

  //! Checks if the one of the nieghbors of the voxel is a background voxel
  /**
    /param[in] voxel_ind      the index of the voxel
    /return true is the at least of the nieghbors of the voxel is
                 in the background
  */
  bool has_background_neighbor(long voxel_ind);

  //! Finds all of the voxels that are part of the surface
  //! (i.e, seperate background from scene)
  /**
     /param[in] shell indexes of all of the surface particles
                      will be stored here.
  */
  void set_surface_shell(std::vector<long> *shell);

  void set_neighbor_mask();
  float surface_val_;
  std::vector<long> neighbor_shift_;
  std::vector<emreal> neighbor_dist_;
  int num_shells_;
};
//TODO : add a static function that given particles returns
//the particles on the surface, we can give it to non-bonded.
#endif // _SURFACE_SHELL_DENSITY_MAP_H
