#ifndef _DENSITYHEADER_H
#define _DENSITYHEADER_H

#include "EM_config.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include "def.h"
#include "ErrorHandling.h"

/** \todo change so that the att will not be encoded but loaded from
    a conf file (Keren) */
class EMDLLEXPORT DensityHeader
{

public:
  DensityHeader() {

    top_calculated=false;
    // Initialize some of the parameters of the header
    nx=0;ny=0;nz=0; 
    data_type=5; 
    nxstart=1; nystart=1; nzstart=1;

    mx = nxstart + nx - 1; my = nystart + ny - 1; mz = nzstart + nz - 1;
    xorigin=0.0; yorigin=0.0; zorigin =0.0;
    alpha=90. ; beta=90. ; gamma=90.;
    mapc =1; mapr=2; maps=3;
    ispg=0;
    nsymbt=0;
    Objectpixelsize = 1.0;
    machinestamp = 0;
    nlabl=0;
    strcpy(map,"MAP \0");
    magic=6;
  }

  // If a copy constructor is not defined in a class, the compiler itself
  // defines one. This will ensure a shallow copy. 
  // If the class does not have pointer variables with dynamically allocated
  // memory, then one need not worry about defining a 
  // copy constructor. It can be left to the compiler's discretion.
  // But if the class has pointer variables and has some dynamic memory
  // allocations, then it is a must to have a copy constructor.

  //! gets the upper bound of the grid
  /** \param[in] ind The dimension index x:=0,y:=1,z:=2
      \return the coordinate value in angstroms
   */
  emreal get_top(int ind) const 
  { 
    if (!top_calculated) {
      std::cerr << " DensityHeader::get_top  the top coordinates of the map "
                << "have not been setup yet " << std::endl;
      throw 1;
    }
    if (ind==0) return xtop;
    if (ind==1) return ytop;
    return ztop;

  }

  //! Computes the maximum x,y,z coordinates of the grid.
  /**
   */
  void compute_xyz_top();

  friend std::ostream& operator<<(std::ostream& s, const DensityHeader &v) {
    s<< "nx: " << v.nx << " ny: " << v.ny << " nz: " << v.nz << std::endl;
    s<<"data_type: " << v.data_type << std::endl;
    s <<"nxstart: " << v.nxstart << " nystart: " << v.nystart <<" nzstart: "
      << v.nzstart << std::endl;
    s<<"mx: "<< v.mx <<" my:" << v.my << " mz: " << v.mz << std::endl;
    s << "xlen: " << v.xlen <<" ylen: " << v.ylen <<" zlen: " << v.zlen
      << std::endl;
    s <<"alpha : " << v.alpha << " beta: " << v.beta <<" gamma: "<< v.gamma
      << std::endl;
    s << "mapc : " << v.mapc << " mapr: " << v.mapr <<" maps: " << v.maps
      << std::endl;
    s << "dmin: " << v.dmin << " dmax: " << v.dmax << " dmean: " << v.dmean
      << std::endl;
    s <<"ispg: " << v.ispg << std::endl;
    s <<"nsymbt: " << v.nsymbt << std::endl;
    s << "user: " << v.user << std::endl;
    s <<"xorigin: " << v.xorigin << " yorigin: "<< v.yorigin <<" zorigin: "
      << v.zorigin << std::endl;
    s <<"map: " << v.map << std::endl;
    s << "Objectpixelsize: " << v.Objectpixelsize << std::endl;
    s<< "machinestamp: " << v.machinestamp << std::endl;
    s <<"rms: " << v.rms << std::endl;
    s<<"nlabl: " << v.nlabl <<std::endl;
    for(int i=0;i<v.nlabl;i++)
      s << "comments[" << i << "] = ->" <<  v.comments[i] << "<-" << std::endl;
    return s;
  }


  static const unsigned short MAP_FIELD_SIZE   =  4;
  static const unsigned short USER_FIELD_SIZE     =  25;
  static const unsigned short COMMENT_FIELD_NUM_OF     =  10;
  static const unsigned short COMMENT_FIELD_SINGLE_SIZE    =  80;

  int nx,ny,nz; // map size
  //! How many bits are used to store the density of a single voxel
  //! (used in MRC format)
  int data_type;
  int nxstart,nystart,nzstart; //number of first columns in map 
  int mx, my, mz; // Number of intervals along each dimension
  float xlen,ylen,zlen; //Cell dimensions (angstroms) 
  float alpha, beta, gamma; //Cell angles (degrees)
  //! Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
  //! (1,2,3 for x,y,z)
  int mapc, mapr, maps;
  float dmin,dmax,dmean; //Minimum, maximum and mean density value
  int ispg; //space group number 0 or 1
  int nsymbt; //Number of bytes used for symmetry data
  int user[USER_FIELD_SIZE];//extra space used for anything
  char map[MAP_FIELD_SIZE]; //character string 'MAP ' to identify file type
  int machinestamp; //machine stamp (0x11110000 bigendian, 0x44440000 little)
  float rms; //RMS deviation of map from mean density
  int nlabl; //Number of labels being used
  //! text comments \todo MRC: labels[10][80] - should it be a different field?
  char comments[COMMENT_FIELD_NUM_OF][COMMENT_FIELD_SINGLE_SIZE];
  //! magic byte for machine platform (~endian), OS-9=0, VAX=1, Convex=2,
  //! SGI=3, Sun=4, Mac(Motorola)=5, PC,IntelMac=6
  int magic;
  float voltage; //Voltage of electron microscope
  float Cs;  //Cs of microscope
  float Aperture;  //Aperture used
  float Magnification;  //Magnification
  float Postmagnification; //Postmagnification (of energy filter)
  float Exposuretime; //Exposuretime
  float Objectpixelsize; //this is the actual pixelsize 
  float Microscope;  //Microscope
  float Pixelsize; //Pixelsize - used for the microscope CCD camera
  float CCDArea;  //CCDArea
  float Defocus;  //Defocus
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

  //! Returns the resolution of the map
  inline float get_resolution() { return resolution;}
  //! Sets the resolution of the map
  void set_resolution(float resolution_) { resolution=resolution_;}
  //! Returns the origin on the map (x-coordiante)
  inline float get_xorigin() const {return xorigin;}
  //! Returns the origin on the map (y-coordiante)
  inline float get_yorigin() const {return yorigin;}
  //! Returns the origin on the map (z-coordiante)
  inline float get_zorigin() const {return zorigin;}
  //! Returns the origin on the map 
  /**
  \param[in] i the relevant coordinate (0:x, 1:y, 2:z)
  \exception EMBED_WrongValue if the value of i is out of range.
  */
  inline float get_origin(int i) const {
    if (not (i>=0 && i<3)) {
      std::ostringstream msg;
      msg << "DensityHeader::get_origin >> the input parameter i is out of"
      <<" range , should be between 0-2 and the value is " << i << "\n";
      throw EMBED_WrongValue(msg.str().c_str());
    }
    switch (i) {
      case 0: return get_xorigin();
      case 1: return get_yorigin();
      default: return get_zorigin();
    }
  }
  //! Sets the origin on the map (x-coordiante)
  inline void set_xorigin(float x)  {xorigin=x; top_calculated=false;}
  //! Sets the origin on the map (y-coordiante)
  inline void set_yorigin(float y)  {yorigin=y; top_calculated=false;}
  //! Sets the origin on the map (z-coordiante)
  inline void set_zorigin(float z)  {zorigin=z; top_calculated=false;}
  //! True if the top coodinates (bounding-box) are calculated
  inline bool is_top_calculated() const { return top_calculated;}

protected:
  float xtop, ytop,ztop; // The upper bound for the x,y and z grid.
  float xorigin, yorigin, zorigin; //Origin used for transforms
  bool top_calculated;
  float resolution;
};


#endif 
