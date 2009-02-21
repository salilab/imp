#include "SurfaceShellDensityMap.h"
#include "limits.h"

SurfaceShellDensityMap::SurfaceShellDensityMap() : SampledDensityMap() {
}

SurfaceShellDensityMap::SurfaceShellDensityMap(const DensityHeader &header)
                           : SampledDensityMap(header) {
  set_neighbor_mask();
}

SurfaceShellDensityMap::SurfaceShellDensityMap(
      const ParticlesAccessPoint &access_p,
      float voxel_size,int num_shells)
  :SampledDensityMap(access_p,_RESOLUTION,voxel_size,_SIG_CUTOFF)
{
  num_shells_=num_shells;
  set_neighbor_mask();
}


//TODO : think about the values for delta as a function of resolution
void SurfaceShellDensityMap::set_neighbor_mask() {
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        if (x == 0 && y == 0 && z == 0)
          continue;
        neighbor_shift_.push_back(
              z * header_.nx * header_.ny + y * header_.nx + x);
        neighbor_dist_.push_back(
            header_.Objectpixelsize * sqrt((1.0*x*x + y*y + z*z)));
      }
    }
  }
}
// TODO: binaries should be a special case of resample, either make
//template or pass pointer to voxel update function
// TODO: pass the background value as well to the general resample function
//TODO: make this function faster
void SurfaceShellDensityMap::binaries(const ParticlesAccessPoint &access_p,
                                 float scene_val) {
  reset_data(_BACKGROUND_VAL);
  calc_all_voxel2loc();
  int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;

  // actual sampling
  float tmpx,tmpy,tmpz;
  int nxny=header_.nx*header_.ny;
  int znxny;
  float rsq,tmp;

  const  KernelParameters::Parameters *params;
  for (int ii=0; ii<access_p.get_size(); ii++) {
    // compute kernel parameters if needed
    try {
      params = kernel_params_.find_params(access_p.get_r(ii));
    }
    catch (EMBED_LogicError &e){
      kernel_params_.set_params(access_p.get_r(ii));
      params = kernel_params_.find_params(access_p.get_r(ii));
    }

    // compute the box affected by each particle
    calc_sampling_bounding_box(access_p.get_x(ii), access_p.get_y(ii),
                               access_p.get_z(ii), params->get_kdist(),
                               iminx, iminy, iminz, imaxx, imaxy, imaxz);
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      znxny=ivoxz * nxny;
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
        // we increment ivox this way to avoid unneceessary multiplication
        // operations.
        ivox = znxny + ivoxy * header_.nx + iminx;
        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          tmpx=x_loc_[ivox] - access_p.get_x(ii);
          tmpy=y_loc_[ivox] - access_p.get_y(ii);
          tmpz=z_loc_[ivox] - access_p.get_z(ii);
          rsq = tmpx*tmpx+tmpy*tmpy+tmpz*tmpz;
          tmp = EXP(-rsq * params->get_inv_sigsq());
          //tmp = exp(-rsq * params->get_inv_sigsq());
          // if statement to ensure even sampling within the box
          if (tmp>kernel_params_.get_lim())
            data_[ivox]= scene_val;
          ivox++;
        }
      }
    }
  }

  // The values of dmean, dmin,dmax, and rms have changed
  rms_calculated_ = false;
  normalized_ = false;
}


bool SurfaceShellDensityMap::has_background_neighbor(long voxel_ind) {
  long n_voxel_ind;
  long num_voxels = header_.nx * header_.ny * header_.nz;
  for (unsigned int j = 0; j < neighbor_shift_.size(); j++) {
    n_voxel_ind = voxel_ind + neighbor_shift_[j];
    if ((n_voxel_ind>-1) && (n_voxel_ind<num_voxels)) {
      if (data_[n_voxel_ind] == _BACKGROUND_VAL) {
        return true;
      }
    }
  }
  return false;
}

void SurfaceShellDensityMap::set_surface_shell(std::vector<long> *shell) {
  //a voxel is part of the outher shell if it has at least one
  //background nieghbor
  for(long i=0;i<get_number_of_voxels();i++) {
    if (has_background_neighbor(i)) {
      data_[i] = _SURFACE_VAL;
      shell->push_back(i);
    }
  }
  //TODO - add some radius to the surface ( not sure it is really needed)
}

// Computes distance function for the molecule.
// The voxels corresponding to surface points are given zero distance
void SurfaceShellDensityMap::resample(const ParticlesAccessPoint &access_p) {
  //all scene voxels will be assigned the value -background_val_
  //(which is positive and larger than 0)
  //TODO - change here, the value of the inner voxels should note be
  //should not be ns*2 but the largest of the inner shell
  binaries(access_p,num_shells_*2);

  //find the voxeles that are part of the surface, so we'll have
  //background, surface and interior voxels
  std::vector<long> curr_shell_voxels;
  //all of the voxels that are part of the current shell
  set_surface_shell(&curr_shell_voxels);
  //all of the voxels that are part of the next shell
  std::vector<long> next_shell_voxels;
  //keeps the shell index for each of the data voxels
  std::vector<int> shell_voxels;
  shell_voxels.insert(shell_voxels.end(),get_number_of_voxels(),-1);
  for(long i=0;i<get_number_of_voxels();i++) {
    if (data_[i] == _SURFACE_VAL) {
      shell_voxels[i]=0;
    }
  }
  long n_voxel_ind,voxel_ind;
  float dist_from_surface; //the value is the distance of the voxel
                           //from the surface
  std::vector<long> *curr_p = &curr_shell_voxels;
  std::vector<long> *next_p = &next_shell_voxels;
  std::vector<long> *tmp_p;
  long num_voxels = get_number_of_voxels();
  for (int s_ind = 0; s_ind <num_shells_; s_ind++) {
    // update voxels with current layer distance and insert indexes
    //for next shell
    for(std::vector<long>::iterator it =  curr_p->begin();
                                    it != curr_p->end();it++) {
      voxel_ind = *it;
      for (unsigned int j = 0; j < neighbor_shift_.size(); j++) {
        n_voxel_ind = voxel_ind + neighbor_shift_[j];
                     //the index of the neighbor
        if ((n_voxel_ind>-1)&&(n_voxel_ind<num_voxels)) {
            dist_from_surface = data_[voxel_ind] + neighbor_dist_[j];
            //if the stored distance of the voxel (voxel_ind) from the surface
            //is larger than the current calculated one, update
            if (data_[n_voxel_ind] > dist_from_surface) {
              data_[n_voxel_ind] = dist_from_surface;
              // set the voxels for the next shell
              if (shell_voxels[n_voxel_ind] < s_ind + 1) {
                next_p->push_back(n_voxel_ind);
                shell_voxels[n_voxel_ind] = s_ind + 1;
              }
            }
         }
       }
    }
    curr_p->clear();
    tmp_p = curr_p; curr_p = next_p; next_p = tmp_p;
  }
}
