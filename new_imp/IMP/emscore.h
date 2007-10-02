#ifndef __EMSCORE__
#define __EMSCORE__

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include "emfile.h"

#include "IMP_config.h"

/* coordinates of model */
struct coord {
  float *x;
  float *y;
  float *z;
  int nc;
};

// get coordinates of grid points given dims and pixelsize
IMPDLLEXPORT
struct gridcoord *em2gridcoord(int nx, int ny, int nz, float pixelsize,
                                     float orig_x, float orig_y, float orig_z, int *ierr);

IMPDLLEXPORT
int get_closestvox(float x, float y, float z, struct gridcoord *gridcd, int *ierr);

// actual em-score
IMPDLLEXPORT
float emscore (struct density *emdens, int nx, int ny, int nz, float pixelsize, float resolution,
               float *cdx, float *cdy, float *cdz, float *dvx, float *dvy, float *dvz, int ncd,
               float *radius, float *wei, struct gridcoord *gridcd, float scalefac, int lderiv, int *ierr);

IMPDLLEXPORT
void deriv_emscore (struct density *emdens, int nx, int ny, int nz, float pixelsize,
                    float resolution, float *cdx, float *cdy, float *cdz,
                    float *dvx, float *dvy, float *dvz, int ncd,
                    float *radius, float *wei, struct gridcoord *gridcd, float scalefac, int *ierr);

IMPDLLEXPORT
float temscore (struct density *emdens, int nx, int ny, int nz, float pixelsize,
                float resolution, struct gridcoord *gridcd, float scalefac, int *ierr);

// temporary score for hardcoded model
IMPDLLEXPORT
struct density *modsample (int nx, int ny, int nz, float pixelsize,
                                 float resolution, float *cdx, float *cdy, float *cdz, int ncd,
                                 float *radius, float *wei, struct gridcoord *gridcd, int *ierr);

IMPDLLEXPORT
struct density *tmodsample (int nx, int ny, int nz, float pixelsize,
                                  float resolution, struct gridcoord *gridcd, int *ierr);

IMPDLLEXPORT
float stdv (struct density *emdens, float *meanval);

IMPDLLEXPORT
void stdnormalize(struct density *emdens);

#endif
