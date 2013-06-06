/**
 *  \file fft_fitting_utils.cpp   \brief masking function for fft fitting
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */


#include <IMP/multifit/internal/fft_fitting_utils.h>
#include <IMP/constants.h>
IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE
  double* convolve_array (double *in_arr,
                          unsigned int nx,
                          unsigned int ny,
                          unsigned int nz,
                          double *kernel,
                          unsigned int nk){
  int margin=(nk-1)/2;
  long n_out=nx*ny*nz;
  double *out_arr = new double[n_out];
  for(int i=0;i<n_out;i++) {
    out_arr[i]=0.;
  }

  double val;
  for (int indz=margin;indz<(int)nz-margin;indz++)
    for (int indy=margin;indy<(int)ny-margin;indy++)
      for (int indx=margin;indx<(int)nx-margin;indx++) {
        val = in_arr[indz*nx*ny+indy*nx+indx];
        if ((val < em::EPS) && (val > -em::EPS))
          continue;
        for (int zz=-margin;zz<=margin;zz++)
          for (int yy=-margin;yy<=margin;yy++)
            for (int xx=-margin;xx<=margin;xx++) {
              out_arr[(indz+zz)*nx*ny+(indy+yy)*nx+indx+xx]+=
                kernel[(zz+margin)*nk*nk+(yy+margin)*nk+xx+margin]*val;
            }}
  return out_arr;
}
//! Generates a nondegenerate set of Euler angles with a delta resolution
EulerAnglesList get_uniformly_sampled_rotations(
                      double angle_sampling_internal_rad) {
  double delta= angle_sampling_internal_rad/IMP::PI*180.0;
  algebra::Vector3D eu_start(0.,0.,0.);//psi,theta,phi
  algebra::Vector3D eu_end(360.,180.,360.);
  algebra::Vector3D eu_range=eu_end-eu_start;
  double phi_steps       = algebra::get_rounded(eu_range[2]/delta+0.499);
  double phi_real_dist   = eu_range[2] / phi_steps;
  double theta_steps     = algebra::get_rounded(eu_range[1]/delta+0.499);
  double theta_real_dist = eu_range[1] / theta_steps;
  double angle2rad=PI/180.;
  double psi_steps,psi_ang_dist,psi_real_dist;

  EulerAnglesList ret;
  for (double phi=eu_start[2]; phi < eu_end[2];  phi+=phi_real_dist) {
    for (double theta=eu_start[1]; theta <= eu_end[1]; theta+=theta_real_dist) {
      if (theta == 0.0 || theta == 180.0) {
        psi_steps = 1;
      }
      else {
        psi_steps =
          algebra::get_rounded(360.0*std::cos((90.0-theta)*angle2rad)/delta);
      }
      psi_ang_dist  = 360.0/psi_steps;
      psi_real_dist = eu_range[0] / (ceil(eu_range[0]/psi_ang_dist));
      for (double psi=eu_start[0]; psi < eu_end[0];  psi+=psi_real_dist)  {
        ret.push_back(EulerAngles(
                                            psi*angle2rad,
                                            theta*angle2rad,
                                            phi*angle2rad));
      }}}
  return ret;
}

void convolve_kernel_inside_erode (em::DensityMap *dmap,
                                   double *kernel, unsigned kernel_size) {
  base::Pointer<em::DensityMap> copy_map=em::create_density_map(dmap);
  copy_map->set_was_used(true);
  int nx=dmap->get_header()->get_nx();
  int ny=dmap->get_header()->get_ny();
  int nz=dmap->get_header()->get_nz();
  int margin=(kernel_size-1)/2;
  dmap->reset_data(0.);
  em::emreal *data=dmap->get_data();
  em::emreal *copy_data=copy_map->get_data();

  for (int indz=margin;indz<nz-margin;indz++)
    for (int indy=margin;indy<ny-margin;indy++)
      for (int indx=margin;indx<nx-margin;indx++) {
        int skip = 0;
        for (int indz2=-margin;skip==0&&indz2<=margin;indz2++)
          for (int indy2=-margin;skip==0&&indy2<=margin;indy2++)
            for (int indx2=-margin;skip==0&&indx2<=margin;indx2++) {
              // check if kernel hits zero density
              double dval=
                copy_data[(indz+indz2)*ny*nx+(indy+indy2)*nx+indx+indx2];
              double dval2 = kernel[(margin-indz2)*kernel_size*kernel_size+
                                    (margin-indy2)*kernel_size+margin-indx2];
              if (dval == 0.0 && dval2 != 0.0) skip = 1;
            }
        if (skip == 0) {
          for (int indz2=-margin;indz2<=margin;indz2++)
            for (int indy2=-margin;indy2<=margin;indy2++)
              for (int indx2=-margin;indx2<=margin;indx2++) {
                double dval=
                  copy_data[(indz+indz2)*ny*nx+(indy+indy2)*nx+indx+indx2];
                double dval2 =
                  kernel[(margin-indz2)*kernel_size*kernel_size+
                         (margin-indy2)*kernel_size+margin-indx2];
                data[indz*ny*nx+indy*nx+indx] += dval * dval2;
              }
        }}}

void relax_laplacian (em::DensityMap *dmap,
                      unsigned ignored[3], double radius){
  int extx=dmap->get_header()->get_nx();
  int exty=dmap->get_header()->get_ny();
  int extz=dmap->get_header()->get_nz();
  double average[27]={0.0,0.0,0.0,0.0,1.0/6.0,0.0,0.0,0.0,0.0,0.0,
                      1.0/6.0,0.0,1.0/6.0,0.0,1.0/6.0,0.0,1.0/6.0,
                      0.0,0.0,0.0,0.0,0.0,1.0/6.0,0.0,0.0,0.0,0.0};
  int margx = (int)(ignored[0]+radius);
  int margy = (int)(ignored[1]+radius);
  int margz = (int)(ignored[2]+radius);
  int margin = (int) ceil(radius);

  // allocate phi mask
  unsigned long nvox = extx * exty * extz;
  char *mask = (char *) malloc(nvox * sizeof(char));
  em::emreal *data=dmap->get_data();
  IMP_INTERNAL_CHECK(mask != nullptr,"bad vector allocation\n");
  unsigned long indv;
  for (indv=0;indv<nvox;indv++) *(mask+indv)=1;
  // assign phi mask value based on distance to thresholded map
  for (int indz=margz;indz<extz-margz;indz++)
    for (int indy=margy;indy<exty-margy;indy++)
      for (int indx=margx;indx<extx-margx;indx++) {
        indv = indz*margy*margx+indy*margx+indx;
        if (data[indv]!=0)
          for (int indz2=-margin;indz2<=margin;indz2++)
            for (int indy2=-margin;indy2<=margin;indy2++)
              for (int indx2=-margin;indx2<=margin;indx2++) {
                unsigned long indw =
                  (indz+indz2)*margy*margx+(indy+indy2)*margx+indx+indx2;
                if (data[indw]==0.0
                    && indz2*indz2+indy2*indy2+indx2*indx2<radius*radius)
                  *(mask+indw)=0;
              }}
  // compute norm
  unsigned long maskcount = 0;
  unsigned long threscount = 0;
  double norm = 0.0;
  for (indv=0;indv<nvox;indv++) {
    if (data[indv]!=0.0) {
      ++threscount;
      norm += data[indv];
    } else if (*(mask+indv)==0) ++maskcount;
  }
  norm /= (double)threscount;// average density for thresholded volume
  norm *= maskcount;
  // iterate on original lattice, no focusing
  base::Pointer<em::DensityMap> copy_map=em::create_density_map(dmap);
  copy_map->set_was_used(true);
  double diff;
  em::emreal *copy_data=copy_map->get_data();
  do {
    copy_map->convolute_kernel(dmap,average,3);
    diff=0.0;
    for (int indz=ignored[2];indz<(int)(extz-ignored[2]);indz++)
      for (int indy=ignored[1];indy<(int)(exty-ignored[1]);indy++)
        for (int indx=ignored[0];indx<(int)(extx-ignored[0]);indx++) {
          unsigned long indv = indz*extx*exty+indy*extx+indx;
          if (*(mask+indv)==0) {
            diff += fabs(copy_data[indv] - data[indv]);
            data[indv]=copy_data[indv];
          }
        }
  } while (diff > 1E-8 * norm);
  free(mask);
  }

  void get_rotation_matrix (double m[3][3],
                       double psi, double theta, double phi) {
    double s1 = std::sin(psi);
    double c1 = std::cos(psi);
    double s2 = std::sin(theta);
    double c2 = std::cos(theta);
    double s3 = std::sin(phi);
    double c3 = std::cos(phi);
    m[0][0] = c1 * c3 - c2 * s3 * s1;
    m[0][1] = c1 * s3 + c2 * c3 * s1;
    m[0][2] = s1 * s2;
    m[1][0] = -s1 * c3- c2 * s3 * c1;
    m[1][1] = -s1 * s3+ c2 * c3 * c1;
    m[1][2] =  c1 * s2;
    m[2][0] =  s2 * s3;
    m[2][1] = -s2 * c3;
    m[2][2] =  c2;
  }
  void rotate_mol(atom::Hierarchy mh,double psi,double theta,double phi) {
    core::XYZs ps = core::XYZs(core::get_leaves(mh));
    double m[3][3];
    get_rotation_matrix(m,psi,theta,phi);
    algebra::Vector3D curr;
    for(unsigned int i=0;i<ps.size();i++) {
      curr=ps[i].get_coordinates();
      double currx=curr[0];
      double curry=curr[1];
      double currz=curr[2];
      ps[i].set_coordinates(algebra::Vector3D(
         currx * m[0][0] + curry * m[0][1] + currz * m[0][2],
         currx * m[1][0] + curry * m[1][1] + currz * m[1][2],
         currx * m[2][0] + curry * m[2][1] + currz * m[2][2]));
    }
  }

  void translate_mol(atom::Hierarchy mh,algebra::Vector3D t) {
    core::XYZs ps = core::XYZs(core::get_leaves(mh));
    algebra::Vector3D curr;
    for(unsigned int i=0;i<ps.size();i++) {
      curr=ps[i].get_coordinates();
      double currx=curr[0];
      double curry=curr[1];
      double currz=curr[2];
      ps[i].set_coordinates(algebra::Vector3D(
                                              currx+t[0],
                                              curry+t[1],
                                              currz+t[2]));
    }
  }

IMPMULTIFIT_END_INTERNAL_NAMESPACE
