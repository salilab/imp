#ifndef _MRCREADERWRITER_H
#define _MRCREADERWRITER_H

/*
  CLASS

  KEYWORDS

  AUTHORS


  Javier Velazquez-Muriel

  OVERVIEW TEXT

*/


#include "EM_config.h"
#include "MapReaderWriter.h"
#include <string>


#define MRC_LABEL_SIZE      80
#define MRC_USER            25
#define MRC_NUM_LABELS      10



/** MRC header */
class EMDLLEXPORT MRCHeader
{
public:
  int nx,ny,nz; // map size
  int mode; /* 0 image : signed 8-bit bytes range -128 to 127.
               1 image : 16-bit halfwords. 
               2 image : 32-bit reals.
               3 transform : complex 16-bit integers
               4 transform : complex 32-bit reals */
  int nxstart,nystart,nzstart; // number of first columns in map (default = 0)
  int mx, my, mz;  /* Number of intervals along each dimension */
  float xlen,ylen,zlen; // Cell dimensions (angstroms)
  float alpha, beta, gamma; // Cell angles (degrees)
  /** Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
      (1,2,3 for x,y,z) */
  int mapc, mapr, maps;
  /** Minimum, maximum and mean density value */
  float dmin,dmax,dmean;
  int ispg; // Sapce group number 0 or 1 (default 0) 
  int nsymbt; // Number of bytes used for symmetry data (0 or 80)
  int user[MRC_USER]; // extra space used for anything - 0 by default
  float xorigin, yorigin, zorigin; // Origin used for transforms 
  char map[4]; // character string 'MAP ' to identify file type
  int machinestamp; // machine stamp (0x11110000 bigendian, 0x44440000 little)
  float rms; // RMS deviation of map from mean density
  int nlabl; // Number of labels being used
  char labels[MRC_NUM_LABELS][MRC_LABEL_SIZE]; // text labels
public:
  void FromDensityHeader(const DensityHeader &h);
  void ToDensityHeader(DensityHeader &h);

  //! Outputs coordinates delimited by single space.
  friend std::ostream& operator<<(std::ostream& s, const MRCHeader &v) {
    s<< "nx: " << v.nx << " ny: " << v.ny << " nz: " << v.nz << std::endl;
    s<<"mode: " << v.mode << std::endl;
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
    s << "xorigin: " << v.xorigin << " yorigin: "<< v.yorigin
      << " zorigin: "<< v.zorigin << std::endl;
    s <<"map: " << v.map << std::endl;
    s<< "machinestamp: " << v.machinestamp << std::endl;
    s <<"rms: " << v.rms << std::endl;
    s<<"nlabl: " << v.nlabl << std::endl;
    s <<"labels : " << v.labels << std::endl;
    for(int i=0;i<v.nlabl;i++)
      s <<"labels[" << i << "] = ->" <<  v.labels[i] << "<-" << std::endl;
    return s;
  }



};


class EMDLLEXPORT MRCReaderWriter : public MapReaderWriter
{
public:

  MRCReaderWriter(void){}
  MRCReaderWriter(char *fn) {
    filename=fn;
  }
  //! Reads an MRC file and translates the header to the general DensityHeader 
  void Read(const char *fn_in, float **data, DensityHeader &head);
  //! Writes an MRC file from the data and the general DensityHeader
  void Write(const char *fn_out, const float *data, const DensityHeader &head);


private:


  // By default the data are read into the grid of the class, but an external
  // pointer to another grid can be specified
  void read(void) {
    read(&grid);
  }
  void read(float **pt);

  //! read the header
  void read_header(void);
  //! read different modes
  void read_data(float *pt);
  void read_8_data(float *pt);
  void read_32_data(float *pt);
  void read_grid(void *pt,size_t size,size_t n);
  void seek_to_data(void);
  // Write functions
  void write(const char *fn) {
    return write(fn,grid);
  }

  void write(const char *fn,const float *pt);
  void write_header(std::ofstream &s);
  void write_data(std::ofstream &s, const float *pt);


  std::string filename; // Name of the file
  std::fstream fs;  // file stream for the file read
  MRCHeader header; // The header of the file
  // The grid of data. The data is stored in the grid with the convention
  // that the order of indexes is z,y,x
  float *grid;

};



EMDLLEXPORT
//!Returns a CCP4 convention machine stamp: 0x11110000 for big endian, 
//!or 0x44440000 for little endian 
int get_machine_stamp(void);

EMDLLEXPORT
//! Returns true if this machine is big endian
int is_bigendian(void);

EMDLLEXPORT
//! Swaps the byte order in an array of 32-bit ints
void byte_swap(unsigned char *ch, int n_array);


#endif //_MRCREADERWRITER_H
