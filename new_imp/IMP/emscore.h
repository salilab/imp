#ifndef __EMSCORE__
#define __EMSCORE__

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include "emfile.h"

/* coordinates of model */
struct coord {
  float *x;
  float *y;
  float *z;
  int nc;
};

// get coordinates of grid points given dims and pixelsize
struct gridcoord *em2gridcoord(int nx, int ny, int nz, float pixelsize,
                                     float orig_x, float orig_y, float orig_z, int *ierr);
int get_closestvox(float x, float y, float z, struct gridcoord *gridcd, int *ierr);
// actual em-score
float emscore (struct density *emdens, int nx, int ny, int nz, float pixelsize, float resolution,
               float *cdx, float *cdy, float *cdz, float *dvx, float *dvy, float *dvz, int ncd,
               float *radius, float *wei, struct gridcoord *gridcd, float scalefac, int lderiv, int *ierr);
void deriv_emscore (struct density *emdens, int nx, int ny, int nz, float pixelsize,
                    float resolution, float *cdx, float *cdy, float *cdz,
                    float *dvx, float *dvy, float *dvz, int ncd,
                    float *radius, float *wei, struct gridcoord *gridcd, float scalefac, int *ierr);
float temscore (struct density *emdens, int nx, int ny, int nz, float pixelsize,
                float resolution, struct gridcoord *gridcd, float scalefac, int *ierr); // temporary score for hardcoded model
struct density *modsample (int nx, int ny, int nz, float pixelsize,
                                 float resolution, float *cdx, float *cdy, float *cdz, int ncd,
                                 float *radius, float *wei, struct gridcoord *gridcd, int *ierr);
struct density *tmodsample (int nx, int ny, int nz, float pixelsize,
                                  float resolution, struct gridcoord *gridcd, int *ierr);
float stdv (struct density *emdens, float *meanval);
void stdnormalize(struct density *emdens);

#endif
