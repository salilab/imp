#ifndef _EMREADWRITE_H
#define _EMREADWRITE_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Friedrich Foerster


  OVERVIEW TEXT

*/

#include "MapReaderWriter.h"
#include "DensityHeader.h"



class EMHeader {
public:
  EMHeader(){}
  EMHeader(const DensityHeader &header);
  void GenerateCommonHeader(DensityHeader &header);


  //attributes
  int nx,ny,nz;
  int magic;
  int type;
  //  int  dimx,dimy,dimz; - todo - ask frido - does not appear in the table
  char comment[80];
  float voltage;
  float Cs;
  float Aperture;
  float Magnification;
  float Postmagnification;
  float Exposuretime;
  float Objectpixelsize;
  float Microscope;
  float Pixelsize;
  float CCDArea;
  float Defocus;
  float Astigmatism;
  float AstigmatismAngle;
  float FocusIncrement;
  float CountsPerElectron;
  float Intensity;
  float EnergySlitwidth;
  float EnergyOffset;
  float Tiltangle;
  float Tiltaxis;
  float MarkerX;
  float MarkerY;
  int lswap;

};


class EMReaderWriter : public MapReaderWriter {
public:
  int Read(ifstream &file, real **data, DensityHeader &header);
  void Write(ostream& s,const real *data, const DensityHeader &header );
protected:
  int ReadHeader(ifstream &file,EMHeader &header);
  int ReadData(ifstream &file, real **data, const EMHeader &header);
  int WriteHeader(ostream& s, const EMHeader &header );
};





#endif //_EMREADWRITE_H
