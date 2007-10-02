#include "emfile.h"

/* free memory of density */
void free_density(struct density *emdens)
{
  free(emdens->data);
  free(emdens);
}

/* get density value from density pointer  */
float get_densityval(struct density *emdens, int x, int y, int z)
{
  float denval;
  int ii;

  ii = z * emdens->nx * emdens->ny + y*emdens->nx + x;
  denval = emdens->data[ii];
  return denval;
};

/* set density value in density pointer  */
void set_densityval(struct density *emdens, int x, int y, int z, float denval)
{
  int ii;

  ii = z * emdens->nx * emdens->ny + y*emdens->nx + x;
  emdens->data[ii] = denval;
};

/* read EM file */
struct density *read_em (char *filename,
                               int *magic,
                               int *type,
                               int *dimx,
                               int *dimy,
                               int *dimz,
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
                               int lswap) {
  struct density *emdens;

  read_em_header (filename, magic, type, dimx, dimy, dimz, comment, voltage,
                  Cs, Aperture, Magnification, Postmagnification, Exposuretime, Objectpixelsize,
                  Microscope, Pixelsize, CCDArea, Defocus, Astigmatism, AstigmatismAngle,
                  FocusIncrement, CountsPerElectron, Intensity, EnergySlitwidth, EnergyOffset,
                  Tiltangle, Tiltaxis, Marker_X, Marker_Y, lswap);
  emdens = read_emdens(filename, *type, *dimx, *dimy, *dimz, lswap);
  return emdens;
}

/* read em-header */
void read_em_header (char *filename,
                     int *magic,
                     int *type,
                     int *dimx,
                     int *dimy,
                     int *dimz,
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
                     int lswap)
{
  int emdata[40];
  unsigned char tchar1[1];
  unsigned char tchar2[2];
  char dummyb[256];
  FILE *input = 0;

  input=fopen(filename, "r");
  fread (tchar1, 1, 1, input);
  *magic = tchar1[0];
  fread (tchar2, 1, 2, input);
  fread (tchar1, 1, 1, input);
  *type = tchar1[0];
  fread (dimx, 4, 1, input);
  fread (dimy, 4, 1, input);
  fread (dimz, 4, 1, input);
  *comment = (char *)  malloc(81);
  fread (*comment, 1, 80, input);
  (*comment)[80] = '\0';
  fread (emdata, 4, 40, input);
  fread (dummyb, 1, 256, input);
  /* copy to output vars  */
  *voltage = emdata[0];
  *Cs = emdata[1];
  *Cs /= 1000.;
  *Aperture = emdata[2];
  *Magnification = emdata[3];
  *Postmagnification = emdata[4];
  *Postmagnification = *Postmagnification / 1000.;
  *Exposuretime = emdata[5];
  *Exposuretime = *Exposuretime/1000.;
  *Objectpixelsize = emdata[6];
  *Objectpixelsize = *Objectpixelsize/1000.;
  *Microscope = emdata[7]+1;
  *Pixelsize = emdata[8];
  *Pixelsize= *Pixelsize/1000.;
  *CCDArea = emdata[9];
  *CCDArea=*CCDArea/1000.;
  *Defocus = emdata[10];
  *Astigmatism = emdata[11];
  *AstigmatismAngle = emdata[12];
  *AstigmatismAngle= *AstigmatismAngle/1000.;
  *FocusIncrement = emdata[13];
  *FocusIncrement = *FocusIncrement/1000.;
  *CountsPerElectron = emdata[14];
  *CountsPerElectron = *CountsPerElectron/1000.;
  *Intensity = emdata[15];
  *Intensity = *Intensity/1000.;
  *EnergySlitwidth = emdata[16];
  *EnergyOffset = emdata[17];
  *Tiltangle = emdata[18];
  *Tiltangle= *Tiltangle/1000.;
  *Tiltaxis = emdata[19];
  *Tiltaxis = *Tiltaxis/1000.;
  *Marker_X = emdata[23];
  *Marker_Y = emdata[24];
  fclose (input);
};

/*write em file */
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
               int lswap)
{
  FILE *output = 0;
  int ii, nvox;
  unsigned char chartmp ;
  int inttmp, dimx, dimy, dimz;
  output=fopen(filename, "w");
  dimx = emdens->nx;
  dimy = emdens->ny;
  dimz = emdens->nz;
  nvox = dimx * dimy * dimz;
  write_em_header (output, magic, type, dimx, dimy, dimz, comment, voltage, Cs,
                   Aperture, Magnification, Postmagnification, Exposuretime, Objectpixelsize,
                   Microscope, Pixelsize, CCDArea, Defocus, Astigmatism, AstigmatismAngle,
                   FocusIncrement, CountsPerElectron, Intensity, EnergySlitwidth, EnergyOffset,
                   Tiltangle, Tiltaxis, Marker_X, Marker_Y, lswap);
  if (type == 1) {
    for (ii=0;ii<=nvox;ii++) {
      chartmp = (unsigned char) emdens->data[ii];
      fwrite(&chartmp, 2, 1, output);
    }
  }
  if (type == 2) {
    for (ii=0;ii<=nvox;ii++) {
      inttmp = (int) emdens->data[ii];
      fwrite(&inttmp, 4, 1, output);
    }
  }
  if (type == 5) {
    fwrite(emdens->data, 4, nvox, output);
  }
  fclose(output);
}

/*write em header to filehandle output */
void write_em_header (FILE *output,
                      int magic,
                      int type,
                      int dimx,
                      int dimy,
                      int dimz,
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
                      int lswap)
{
  int emdata[40];
  unsigned char tchar1;
  unsigned char tchar2[2];
  char dummyb[256];

  tchar1 = (unsigned char) magic;
  fwrite (&tchar1, 1, 1, output);
  fwrite (&tchar2, 1, 2, output);
  tchar1 = (unsigned char) type;
  fwrite (&tchar1, 1, 1, output);
  fwrite (&dimx, 4, 1, output);
  fwrite (&dimy, 4, 1, output);
  fwrite (&dimz, 4, 1, output);
  fwrite (&comment, 1, 80, output);
  emdata[0] = (int) voltage;
  emdata[1] = (int) (Cs*1000.);
  emdata[2] = (int) Aperture;
  emdata[3] = (int) Magnification;
  emdata[4] = (int) (Postmagnification * 1000.);
  emdata[5] = (int) (Exposuretime *1000.);
  emdata[6] = (int) (Objectpixelsize *1000.);
  emdata[7] = (int) (Microscope -1);
  emdata[8] = (int) (Pixelsize *1000.);
  emdata[9] = (int) (CCDArea*1000.);
  emdata[10] = (int) Defocus;
  emdata[11] = (int) Astigmatism ;
  emdata[12] = (int) (AstigmatismAngle * 1000.);
  emdata[13] = (int) (FocusIncrement * 1000.);
  emdata[14] = (int) (CountsPerElectron * 1000.);
  emdata[15] = (int) (Intensity *1000.);
  emdata[16] = (int) (EnergySlitwidth);
  emdata[17] = (int) EnergyOffset;
  emdata[18] = (int) (Tiltangle *1000.);
  emdata[19] = (int) (Tiltaxis *1000.);
  emdata[23] = (int) Marker_X;
  emdata[24] = (int) Marker_Y;
  fwrite (&emdata, 4, 40, output);
  fwrite (&dummyb, 1, 256, output);
};

/* get magic number for endianness  */
void get_magic(char *filename, int *magic)
{
  FILE *input = 0;
  unsigned char dum[1];

  input=fopen(filename, "r");
  fread(dum, 1, 1, input);
  *magic = dum[0];
  fclose (input);
};

/* read in core data */
struct density *read_emdens(char *filename, int type, int nx, int  ny, int nz, int lswap) {
  FILE *input = 0;
  char dummy[512];
  int ii, nvox;
  struct density *emdens;
  unsigned char *bytedata;
  short *intdata;
  float tmp;
  short shorttmp;

  input=fopen(filename, "r");
  /*header info to dummy var  */
  fread (dummy, 1, 512, input);
  nvox = nx*ny*nz;
  emdens = (density *) malloc(sizeof(struct density));
  emdens->data = (float *) malloc(sizeof(float) * nvox);
  emdens->nx = nx;
  emdens->ny = ny;
  emdens->nz = nz;
  if ( type == 1 ) {
    bytedata = (unsigned char *) malloc(sizeof(bytedata)*nvox);
    fread (bytedata, 1, nvox, input);
    for (ii=0;ii<=nvox;ii++) {
      /*if (lswap==1)
        {
          swap((char *) bytedata[ii], 2);
        }*/
      tmp = (float) bytedata[ii];
      emdens->data[ii] = tmp;
    }
    free(bytedata);
  } else if ( type == 2 ) {
    intdata = (short int *) malloc(sizeof(intdata)*nvox);
    fread (intdata, 2, nvox, input);
    for (ii=0;ii<=nvox;ii++) {
      if (lswap==1) {
        shorttmp = intdata[ii];
        swap((char *) &shorttmp, 4);
        intdata[ii] = shorttmp;
      }
      tmp = (float) intdata[ii];
      emdens->data[ii] = tmp;
    }
    free(intdata);
  } else if (type == 5) {
    fread (emdens->data, 4, nvox, input);
    if (lswap==1) {
      for (ii=0;ii<=nvox;ii++) {
        tmp = emdens->data[ii];
        swap((char *) &tmp, 4);
        emdens->data[ii] = tmp;
      }
    }
  } else {
    printf("datatype not implemented \n");
    exit(1);
  }
  fclose(input);
  return emdens;
}

/* swap bytes */
void swap(char *x, char size)
{
  unsigned char c;
  unsigned short s;
  unsigned long l;

  switch (size) {
  case 2: // swap two bytes
    c = *x;
    *x = *(x+1);
    *(x+1) = c;
    break;
  case 4: // swap two shorts (2-byte words)
    s = *(unsigned short *)x;
    *(unsigned short *)x = *((unsigned short *)x + 1);
    *((unsigned short *)x + 1) = s;
    swap ((char *)x, 2);
    swap ((char *)((unsigned short *)x+1), 2);
    break;
  case 8: // swap two longs (4-bytes words)
    l = *(unsigned long *)x;
    *(unsigned long *)x = *((unsigned long *)x + 1);
    *((unsigned long *)x + 1) = l;
    swap ((char *)x, 4);
    swap ((char *)((unsigned long *)x+1), 4);
    break;
  }
}
