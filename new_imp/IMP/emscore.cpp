#include "emscore.h"

// free grid coordinate structure
void free_gridcoord( struct gridcoord *gridcd)
{
  free(gridcd);
};

// get coordinates of grid points given dims and pixelsize
struct gridcoord *em2gridcoord(int nx, int ny, int nz, float pixelsize,
                                     float orig_x, float orig_y, float orig_z, int *ierr) {
  int ii, ix, iy, iz;
  struct gridcoord *gridcd;

  gridcd = (gridcoord *) malloc(sizeof(struct gridcoord));
  gridcd->x = NULL;
  gridcd->y = NULL;
  gridcd->z = NULL;
  gridcd->nc = nx*ny*nz;
  gridcd->orig_x = orig_x;
  gridcd->orig_y = orig_y;
  gridcd->orig_z = orig_z;
  //if (gridcd->x = malloc(sizeof(float) * gridcd->nc) ~= 0){
  //  ierr = 1;
  //  return gridcd;
  //};
  gridcd->x = (float *) malloc(sizeof(float) * gridcd->nc);
  gridcd->y = (float *) malloc(sizeof(float) * gridcd->nc);
  gridcd->z = (float *) malloc(sizeof(float) * gridcd->nc);
  gridcd->pixelsize = pixelsize;
  ix=0;
  iy=0;
  iz=0;
  for (ii=0;ii<gridcd->nc;ii++) {
    //gridcd->x[ii] = orix_x + ix * pixelsize;
    gridcd->x[ii] =  ix * pixelsize;
    //gridcd->y[ii] = orix_y + iy * pixelsize;
    gridcd->y[ii] = iy * pixelsize;
    //gridcd->z[ii] = orix_z + iz * pixelsize;
    gridcd->z[ii] =  iz * pixelsize;
    ix = ix +1;
    if (ix == nx) {
      ix = 0;
      iy = iy+1;
      if (iy == ny) {
        iy = 0;
        iz = iz+1;
      }
    }
  }
  return gridcd;
};

int get_closestvox(float x, float y, float z, struct gridcoord *gridcd, int *ierr)
{
  int ii, ix, iy, iz;

  ix = (int)(x / gridcd->pixelsize + 0.5);
  iy = (int)(y / gridcd->pixelsize + 0.5);
  iz = (int)(z / gridcd->pixelsize + 0.5);
  ii = iz * gridcd->nx * gridcd->ny + iy * gridcd->nx + ix;
  return ii;
};

float stdv (struct density *emdens, float *meanval)
{
  int ii, nvox;
  float stdval, tmp;
  nvox = emdens->nx * emdens->ny * emdens->nz;
  *meanval = .0;
  stdval = .0;
  for (ii=0;ii<nvox;ii++) {
    *meanval = *meanval + emdens->data[ii];
    stdval = stdval + pow(emdens->data[ii], 2);
  }
  *meanval = *meanval / nvox;
  tmp = pow(*meanval, 2) * nvox;
  stdval = stdval - tmp;
  stdval = sqrt(stdval) / nvox;
  emdens->stdval = stdval;
  emdens->meanval = *meanval;
  return stdval;
};

void stdnormalize(struct density *emdens)
{
  float stdval, meanval, inv_stdval;
  int nvox, ii;

  stdval = stdv(emdens, &meanval);
  inv_stdval = 1/stdval;
  nvox = emdens->nx * emdens->ny * emdens->nz;
  for (ii=0;ii<nvox;ii++) {
    emdens->data[ii] = (emdens->data[ii] - meanval) * inv_stdval;
  }
  emdens->stdval=1.;
  emdens->meanval=.0;
}

/* correlation between em density and density of a model moddens
   threshold can be specified that is checked in moddens to reduce
   elements of summation

   emdens    em density (mean=0)
   moddens   density of model
   threshold elements < threshold in moddens are not summed
   emstdval  std dev of em map
   ccnormfac final normalization factor for corr (output) */
float corr (struct density *emdens, struct density *moddens, float threshold,
            float emstdval, float *ccnormfac)
{
  int ii, nvox;
  float modstdval, modmeanval;
  float ccc;

  modstdval = stdv(moddens, &modmeanval);
  nvox = emdens->nx * emdens->ny * emdens->nz;
  ccc = .0;
  for (ii=0;ii<nvox;ii++) {
    if (moddens->data[ii] > threshold) {
      ccc = ccc + moddens->data[ii] * emdens->data[ii];
    }
  }
  printf ("modstdval: %f ccnormfac: %f\n", modstdval, *ccnormfac);
  *ccnormfac = 1./ (nvox * nvox* modstdval * emstdval);
  ccc = ccc/(nvox * nvox* modstdval * emstdval);
  return ccc;
};

float temscore (struct density *emdens, int nx, int ny, int nz, float pixelsize,
                float resolution, struct gridcoord *gridcd, float scalefac, int *ierr)
{
  float cdx[3], cdy[3], cdz[3], dvx[3], dvy[3], dvz[3],radius[3], wei[3];
  int ncd=3, lderiv=1;
  float escore;
  cdx[0]=9.;
  cdx[1]= 9.;
  cdx[2]= 9.;
  cdy[0]=9.;
  cdy[1]= 10.;
  cdy[2]= 2.;
  cdz[0]=9.;
  cdz[1]= 2.;
  cdz[2]= 10.;
  radius[0]=3.0;
  radius[1]= 0.;
  radius[2]= 0.;
  wei[0]=1.;
  wei[1]= 0.;
  wei[2]= 0.;
  escore = emscore (emdens, nx, ny, nz, pixelsize,
                    resolution, &cdx[0], &cdy[0], &cdz[0], &dvx[0], &dvy[0], &dvz[0], ncd,
                    &radius[0], &wei[0], gridcd, scalefac, lderiv, ierr);
  printf("dvx[0]= %f, dvy[0]= %f, dvz[0]= %f \n",dvx[0], dvy[0], dvz[0]);
  return escore;
}

// actual em-score
/*
emdens      em density
nx,ny,nz    dims of density
pixelsize   pixelsize of map in A
resolution  resolution of map in A
cdx,cdy,cdz x,y,z coord of model
dvx,dvy,dvz derivatives of model
ncd         number of coord
radius      radii of each bead at cd
wei         weight of each bead
gridcoord   coordinates of the map
scalefac    scaling factor for score and deriv
lderiv      calc deriv?
ierr        error */
float emscore (struct density *emdens, int nx, int ny, int nz, float pixelsize, float resolution,
               float *cdx, float *cdy, float *cdz, float *dvx, float *dvy, float *dvz, int ncd,
               float *radius, float *wei, struct gridcoord *gridcd, float scalefac, int lderiv, int *ierr)
{
  struct density *moddens;
  float escore, eps=.000001;
  float *ccnormfac;

  *ierr = 0;
  ccnormfac = (float *) malloc(sizeof(float));
  *ccnormfac=.0;
  printf("cdx %f , cdy %f, cdz %f \n", cdx[0], cdy[0], cdz[0]);
  moddens = modsample( nx, ny, nz, pixelsize, resolution, cdx, cdy, cdz, ncd,
                       radius, wei, gridcd, ierr);
  escore = corr (emdens, moddens, eps, emdens->stdval, ccnormfac);
  printf("ccc= %f \n", escore);
  escore = scalefac * (1. - escore);
  free(moddens);
  if (lderiv > 0) {
    deriv_emscore(emdens, nx, ny, nz, pixelsize, resolution, &cdx[0], &cdy[0], &cdz[0],
                  &dvx[0], &dvy[0], &dvz[0], ncd,
                  &radius[0], &wei[0], gridcd, scalefac, ierr);
  }
  return escore;
};

void deriv_emscore (struct density *emdens, int nx, int ny, int nz, float pixelsize,
                    float resolution, float *cdx, float *cdy, float *cdz,
                    float *dvx, float *dvy, float *dvz, int ncd,
                    float *radius, float *wei, struct gridcoord *gridcd, float scalefac, int *ierr)
{
  float lim, sq2pi3, rsig, rsigsq, inv_rsigsq, vsig, vsigsq, sig,
  inv_sigsq, normfac, rnormfac, kdist, rkdist, rsq;
  float tdvx, tdvy, tdvz, tmp;
  float eps=0.000001, pi=3.1415926535897931, timessig=3.;
  int ii, iminx, iminy, iminz, imaxx, imaxy, imaxz, ivox, ivoxx, ivoxy, ivoxz;

  *ierr = 0;
  sq2pi3 = 1. / sqrt(pow(2. * pi, 3));
  // convert resolution to sigma squared
  rsig = 1./(sqrt(2.*log(2.))) * resolution / 2.;
  rsigsq = rsig * rsig;
  inv_rsigsq = 1./rsigsq * .5;
  rnormfac =  sq2pi3 * 1. / pow(rsig, 3.);
  rkdist   = timessig * rsig;
  lim = exp(-0.5 * pow(timessig - eps, 2.));
  for (ii=0; ii<ncd; ii++) {
    if (radius[ii] > eps) {
      vsig = 1./(sqrt(2.*log(2.))) * radius[ii];
      vsigsq = vsig * vsig;
      inv_sigsq = rsigsq + vsigsq;
      sig = sqrt(inv_sigsq);
      kdist = timessig * sig;
      inv_sigsq = 1./inv_sigsq *.5;
      normfac = sq2pi3 * 1.0 / pow(sig, 3.);
    } else {
      inv_sigsq = inv_rsigsq;
      normfac = rnormfac;
      kdist = rkdist;
    };
    iminx = (int)((cdx[ii]-kdist) / gridcd->pixelsize + 0.5);
    if (iminx < 0) iminx = 0;
    iminy = (int)((cdy[ii]-kdist) / gridcd->pixelsize + 0.5);
    if (iminy < 0) iminy = 0;
    iminz = (int)((cdz[ii]-kdist) / gridcd->pixelsize + 0.5);
    if (iminz < 0) iminz = 0;
    imaxx = (int)((cdx[ii]+kdist) / gridcd->pixelsize + 0.5);
    if (imaxx > nx-1) imaxx = nx-1;
    imaxy = (int)((cdy[ii]+kdist) / gridcd->pixelsize + 0.5);
    if (imaxy > ny-1) imaxy = ny-1;
    imaxz = (int)((cdz[ii]+kdist) / gridcd->pixelsize + 0.5);
    if (imaxz > nz-1) imaxz = nz-1;
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * nx * ny + ivoxy * nx + iminx;
        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = pow(gridcd->x[ivox] - cdx[ii], 2)
                + pow(gridcd->y[ivox] - cdy[ii], 2)
                + pow(gridcd->z[ivox] - cdz[ii], 2);
          rsq = exp(- rsq * inv_sigsq );
          tmp = (cdx[ii]-gridcd->x[ivox]) * rsq;
          if ( tmp>lim ) tdvx = tdvx + tmp;
          tmp = (cdy[ii]-gridcd->y[ivox]) * rsq;
          if ( tmp>lim ) tdvy = tdvy + tmp;
          tmp = (cdz[ii]-gridcd->z[ivox]) * rsq;
          if ( tmp>lim ) tdvz = tdvz + tmp;
          ivox++;
        }
      }
    }
    dvx[ii] = dvx[ii] + wei[ii] * 2.*inv_sigsq * scalefac * normfac * tdvx;
    dvy[ii] = dvy[ii] + wei[ii] * 2.*inv_sigsq * scalefac * normfac * tdvy;
    dvz[ii] = dvz[ii] + wei[ii] * 2.*inv_sigsq * scalefac * normfac * tdvz;
  }
};

// temporary routine to avoid messing with swig
struct density *tmodsample (int nx, int ny, int nz, float pixelsize,
                                  float resolution, struct gridcoord *gridcd, int *ierr) {
  float cdx[3], cdy[3], cdz[3], radius[3], wei[3];
  int ncd=3;
  struct density *moddens;
  cdx[0]=9.;
  cdx[1]= 9.;
  cdx[2]= 9.;
  cdy[0]=9.;
  cdy[1]= 10.;
  cdy[2]= 2.;
  cdz[0]=9.;
  cdz[1]= 2.;
  cdz[2]= 10.;
  radius[0]=3.0;
  radius[1]= 0.;
  radius[2]= 0.;
  wei[0]=1.;
  wei[1]= 0.;
  wei[2]= 0.;
  moddens = modsample (nx, ny, nz, pixelsize, resolution, &cdx[0], &cdy[0],
                       &cdz[0], ncd, &radius[0], &wei[0], gridcd, ierr);
  return moddens;
}

/* routine to sample a given model on a grid */
struct density *modsample (int nx, int ny, int nz, float pixelsize,
                                 float resolution, float *cdx, float *cdy, float *cdz, int ncd,
                                 float *radius, float *wei, struct gridcoord *gridcd, int *ierr) {
  int ii, ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  float eps=0.000001, pi=3.1415926535897931, timessig=3.;
  float kdist, rkdist;//kernel distance (= elements for summation)
  float inv_sigsq, rsigsq, rsig, vsigsq, vsig, inv_rsigsq, sig, normfac, rnormfac,
  sq2pi3, rsq, tmp, lim;
  struct density *moddens;

  *ierr = 0;
  sq2pi3 = 1. / sqrt(pow(2. * pi, 3));
  moddens = (density *) malloc(sizeof(struct density));
  moddens->data = (float *) calloc(sizeof(float), nx*ny*nz);
  moddens->nx = nx;
  moddens->ny = ny;
  moddens->nz = nz;
  // convert resolution to sigma squared
  rsig = 1./(sqrt(2.*log(2.))) * resolution / 2.;
  rsigsq = rsig * rsig;
  inv_rsigsq = 1./rsigsq * .5;
  rnormfac = sq2pi3 * 1. / pow(rsig, 3.);
  rkdist   = timessig * rsig;
  lim = exp(-0.5 * pow(timessig - eps, 2.));
  // actual sampling
  for (ii=0; ii<ncd; ii++) {
    if (radius[ii] > eps) {
      vsig = 1./(sqrt(2.*log(2.))) * radius[ii];
      vsigsq = vsig * vsig;
      inv_sigsq = rsigsq + vsigsq;
      sig = sqrt(inv_sigsq);
      kdist = timessig * sig;
      inv_sigsq = 1./inv_sigsq *.5;
      //normfac = sq2pi3 * (1/pow(vsig,3)) * rnormfac;
      normfac = sq2pi3 * 1. / pow(sig, 3.);
    } else {
      inv_sigsq = inv_rsigsq;
      normfac = rnormfac;
      kdist = rkdist;
    };
    iminx = (int)((cdx[ii]-kdist) / gridcd->pixelsize + 0.5);
    if (iminx < 0) iminx = 0;
    if (iminx > nx-1) iminx = nx-1;
    iminy = (int)((cdy[ii]-kdist) / gridcd->pixelsize + 0.5);
    if (iminy < 0) iminy = 0;
    if (iminy > ny-1) iminy = ny-1;
    iminz = (int)((cdz[ii]-kdist) / gridcd->pixelsize + 0.5);
    if (iminz < 0) iminz = 0;
    if (iminz > nz-1) iminz = nz-1;
    imaxx = (int)((cdx[ii]+kdist) / gridcd->pixelsize + 0.5);
    if (imaxx > nx-1) imaxx = nx-1;
    if (imaxx < 0) imaxx = 0;
    imaxy = (int)((cdy[ii]+kdist) / gridcd->pixelsize + 0.5);
    if (imaxy > ny-1) imaxy = ny-1;
    if (imaxy < 0) imaxy = 0;
    //imaxz = int ( (cd->z[ii]+kdist - gridcd->orig_z) / gridcd->pixelsize + 0.5);
    imaxz = (int)((cdz[ii]+kdist) / gridcd->pixelsize + 0.5);
    if (imaxz > nz-1) imaxz = nz-1;
    if (imaxz < 0) imaxz = 0;
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++) {
        ivox = ivoxz * nx * ny + ivoxy * nx + iminx;
        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          rsq = pow(gridcd->x[ivox] - cdx[ii], 2.)
                + pow(gridcd->y[ivox] - cdy[ii], 2.)
                + pow(gridcd->z[ivox] - cdz[ii], 2.);
          tmp = exp(- rsq * inv_sigsq );
          if ( tmp>lim ) moddens->data[ivox] = moddens->data[ivox] + normfac * wei[ii] * tmp;
          ivox++;
        }
      }
    }
  }
  tmp=.0;
  for (ii=0;ii<nx*ny*nz;ii++) {
    tmp = tmp+ moddens->data[ii];
  }
  return moddens;
};

