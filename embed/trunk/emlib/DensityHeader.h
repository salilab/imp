#ifndef _DENSITYHEADER_H
#define _DENSITYHEADER_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT

*/


// TODO - change so that the att will not be encoded but loaded from a conf file (Keren)

class DensityHeader {

public:
  DensityHeader() {}
  DensityHeader(const DensityHeader & other_header) {

    //TODO - add copy const
  }


  static const unsigned short MAP_FIELD_SIZE   =  4;
  static const unsigned short USER_FIELD_SIZE     =  25;
  static const unsigned short COMMENT_FIELD_NUM_OF     =  10;
  static const unsigned short COMMENT_FIELD_SINGLE_SIZE    =  80;



  int nx,ny,nz; // map size
  int data_type; //how many bits are used to store the density of a single voxel
  int nxstart,nystart,nzstart; //number of first columns in map 
  int mx, my, mz; // Number of intervals along each dimension
  float xlen,ylen,zlen; //Cell dimensions (angstroms) 
  float alpha, beta, gamma; //Cell angles (degrees)
  int mapc, mapr, maps; //Axes corresponding to columns (mapc), rows (mapr) and sections (maps) (1,2,3 for x,y,z)
  float dmin,dmax,dmean; //Minimum, maximum and mean density value
  int ispg; //space group number 0 or 1
  int nsymbt; //Number of bytes used for symmetry data
  int user[USER_FIELD_SIZE];//extra space used for anything
  float xorigin, yorigin, zorigin; //Origin used for transforms
  char map[MAP_FIELD_SIZE];	//character string 'MAP ' to identify file type
  int machinestamp; //machine stamp (0x11110000 bigendian, 0x44440000 little)
  float rms; //RMS deviation of map from mean density
  int nlabl; //Number of labels being used
  char comments[COMMENT_FIELD_NUM_OF][COMMENT_FIELD_SINGLE_SIZE]; //text comments TODO: MRC: labels[10][80] - should it be a different field?
  int magic; //magic byte for machine platform (~endian), OS-9=0, VAX=1, Convex=2, SGI=3, Sun=4, Mac(Motorola)=5, PC,IntelMac=6
  float voltage; //Voltage of electron microscope
  float Cs;	//Cs of microscope
  float Aperture;  //Aperture used
  float Magnification;	//Magnification
  float Postmagnification; //Postmagnification (of energy filter)
  float Exposuretime; //Exposuretime
  float Objectpixelsize; //Objectpixelsize
  float Microscope;	//Microscope
  float Pixelsize; //Pixelsize
  float CCDArea;	//CCDArea
  float Defocus;	//Defocus
  float Astigmatism;//Astigmatism
  float AstigmatismAngle; //Astigmatism Angle
  float FocusIncrement;//Focus-Increment
  float CountsPerElectron;//Counts/Electron
  float Intensity;//Intensity
  float EnergySlitwidth;//Energy slitwidth of energy filter
  float EnergyOffset; //Energy offset of Energy filter
  float Tiltangle;//Tiltangle of stage
  float Tiltaxis;//Tiltaxis
  float MarkerX;//Marker_X coordinate
  float MarkerY;//Marker_Y coordinate
  int lswap;
};


#endif 
