#ifndef __EMFILE__
#define __EMFILE__

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

//struct density;
struct density {
  float *data;
  int nx;
  int ny;
  int nz;
  float stdval;// std deviation
  float meanval; // mean value
};

typedef struct density EM_Density;

/* coordinates of em-grid */
struct gridcoord {
  //float *x=NULL;
  //float *y=NULL;
  //float *z=NULL;
  float *x;
  float *y;
  float *z;
  int nx;
  int ny;
  int nz;
  float orig_x;
  float orig_y;
  float orig_z;
  float pixelsize;
  int nc;
};

typedef struct gridcoord EM_Gridcoord;

void read_em_header (char *filename,
                     int *magic,
                     int *type,
                     int  *dimx,
                     int  *dimy,
                     int  *dimz,
                     char **comment,
                     float *voltage,
                     float *Cs,
                     float *Aperture,
                     float *Magnification,
                     float *Postmagnification,
                     float *Exposuretime,
                     float *Objectpixelsize,
                     float *Microscope,
                     float *Pixelsize,
                     float *CCDArea,
                     float *Defocus,
                     float *Astigmatism,
                     float *AstigmatismAngle,
                     float *FocusIncrement,
                     float *CountsPerElectron,
                     float *Intensity,
                     float *EnergySlitwidth,
                     float *EnergyOffset,
                     float *Tiltangle,
                     float *Tiltaxis,
                     float *Marker_X,
                     float *Marker_Y,
                     int lswap);

struct density *read_em (char *filename,
                               int *magic,
                               int *type,
                               int  *dimx,
                               int  *dimy,
                               int  *dimz,
                               char **comment,
                               float *voltage,
                               float *Cs,
                               float *Aperture,
                               float *Magnification,
                               float *Postmagnification,
                               float *Exposuretime,
                               float *Objectpixelsize,
                               float *Microscope,
                               float *Pixelsize,
                               float *CCDArea,
                               float *Defocus,
                               float *Astigmatism,
                               float *AstigmatismAngle,
                               float *FocusIncrement,
                               float *CountsPerElectron,
                               float *Intensity,
                               float *EnergySlitwidth,
                               float *EnergyOffset,
                               float *Tiltangle,
                               float *Tiltaxis,
                               float *Marker_X,
                               float *Marker_Y,
                               int lswap);

void write_em (char *filename,
               int magic,
               int type,
               char comment[80],
               float voltage,
               float Cs,
               float Aperture,
               float Magnification,
               float Postmagnification,
               float Exposuretime,
               float Objectpixelsize,
               float Microscope,
               float Pixelsize,
               float CCDArea,
               float Defocus,
               float Astigmatism,
               float AstigmatismAngle,
               float FocusIncrement,
               float CountsPerElectron,
               float Intensity,
               float EnergySlitwidth,
               float EnergyOffset,
               float Tiltangle,
               float Tiltaxis,
               float Marker_X,
               float Marker_Y,
               struct density *emdens,
               int lswap);

void write_em_header (FILE *output, int magic, int type, int dimx, int dimy, int dimz,
                      char comment[80], float voltage, float Cs, float Aperture, float Magnification,
                      float Postmagnification, float Exposuretime, float Objectpixelsize, float Microscope,
                      float Pixelsize, float CCDArea, float Defocus, float Astigmatism, float AstigmatismAngle,
                      float FocusIncrement, float CountsPerElectron, float Intensity, float EnergySlitwidth,
                      float EnergyOffset, float Tiltangle, float Tiltaxis, float Marker_X, float Marker_Y,
                      int lswap);

void free_density(struct density *emdens);
float get_densityval(struct density *emdens, int x, int y, int z);
void set_densityval(struct density *emdens, int x, int y, int z, float denval);
void get_magic(char *filename, int *magic);
/* read in core data */
struct density *read_emdens(char *filename, int type, int nx, int  ny, int nz, int lswap);
/* swap bytes */
void swap(char *x, char size);

#endif
