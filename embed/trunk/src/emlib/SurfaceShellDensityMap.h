#ifndef _SURFACE_SHELL_DENSITY_MAP_H
#define _SURFACE_SHELL_DENSITY_MAP_H

#include <vector>
#include "EM_config.h"
#include "exp.h"
#include "SampledDensityMap.h"
#include "DensityHeader.h"
#include "def.h"
#include "ParticlesAccessPoint.h"

#define DEFAULT_NUM_SHELLS 5

//! The class repersents a molecule as shells of distance from the surface
/**
 */

class EMDLLEXPORT SurfaceShellDensityMap: public SampledDensityMap 
{

public:


  //! Creates a new density map for sampled map. 
  /** The header of the map is not determined and no data is being allocated
   */
  SurfaceShellDensityMap();

  //! The size of the map is determined by the header and the data is allocated.
  SurfaceShellDensityMap(const DensityHeader &header);

  //! Generatea a sampled density map from the particles.
  /** /param[in] access_p     access point to the particles (locations,
                              radius, weight)
      /param[in] resolution   half width the Gaussian
      /param[in] voxel_size
      /param[in] sig_cutoff   Choose what should be the sigma cutoff for
                 accurate sampling. It is used in two functions;
                 (i)  to determine the size of the grid dimensions
                 (ii) to determine the voxels around the coords participating
                      in the sampling procedure.
   */
  SurfaceShellDensityMap(const ParticlesAccessPoint &access_p, 
                         int num_shells=DEFAULT_NUM_SHELLS,float resolution=4.0,
                         float voxel_size=1.0, int sig_cuttoff=3);

  //! Compute shells.
  /**
  All voxels that are outside of the model defined by the particles will be 
  set to background. 
  Voxels on the surface between the model and the background
  will be set of 0. 
  Voxels inside the model will be divided to num_shells-1, 
  and their positive value will increase as they are furthrer away 
  from the surface.
  */
  void compute_shells(const ParticlesAccessPoint &access_p,
                       int num_shells=DEFAULT_NUM_SHELLS);
  float get_background_value() const {return background_val_;}

protected:
  //! Mark the map voxels as either scene or background
  /**
    /param[in] access_p       the particles of the model (scene)
    /param[in] scene_val      all voxels corredponsing to particles will 
                              be set to this value
    /param[in] background_val all voxels not corredponsing to particles 
                              will be set to this value
   */
  void binaries(const ParticlesAccessPoint &access_p,float scene_val, 
               float background_val);

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

  //! set values of surface and background voxels
  void set_parameters();
  void set_neighbor_mask(emreal delta=1.0); // TODO _ what is delta here ??
  float background_val_;
  float surface_val_;
  std::vector<long> neighbor_shift_;
  std::vector<emreal> neighbor_dist_;
};
//TODO : add a static function that given particles returns 
//the particles on the surface, we can give it to non-bonded.
#endif // _SURFACE_SHELL_DENSITY_MAP_H
