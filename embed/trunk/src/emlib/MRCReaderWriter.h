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



//! Class to deal with the header of MRC files
class EMDLLEXPORT MRCHeader
{
public:
  //! map size (x dimension)
  int nx;
  //! map size (y dimension)
  int ny;
  //! map size (z dimension)
  int nz;
  //! Image mode
  /**
   * 0 image : signed 8-bit bytes range -128 to 127.
   * 1 image : 16-bit halfwords.
   * 2 image : 32-bit reals.
   * 3 transform : complex 16-bit integers
   * 4 transform : complex 32-bit reals
  **/
  int mode;
  //! number of first columns in map (default = 0)
  int nxstart,nystart,nzstart;
  //! Intervals along dimension x
  int mx;
  //! Intervals along dimension y
  int my;
  //! Intervals along dimension z
  int mz;
  //! Cell dimension (angstroms) for x
  float xlen;
  //! Cell dimension (angstroms) for y
  float ylen;
  //! Cell dimension (angstroms) for z
  float zlen;
  //! Cell angle (degrees) for x
  float alpha;
  //! Cell angle (degrees) for y
  float beta;
  //! Cell angle (degrees) for z
  float gamma;
  //! Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
  //!    (1,2,3 for x,y,z)
  int mapc, mapr, maps;
  //! Minimum density value
  float dmin;
  //! Maximum density value
  float dmax;
  //! Mean density value
  float dmean;
  //! Sapce group number 0 or 1 (default 0)
  int ispg;
  int nsymbt;
  //! Number of bytes used for symmetry data (0 or 80)
  int user[MRC_USER];
  //! extra space used for anything - 0 by default
  //! Map origin used for transforms (x dimension)
  float xorigin;
  //! Map origin used for transforms (y dimension)
  float yorigin;
  //! Map origin used for transforms (z dimension)
  float zorigin;
  //! character string 'MAP ' to identify file type
  char map[4];
  //! machine stamp (0x11110000 bigendian, 0x44440000 little)
  int machinestamp;
  //! Standard deviation of map from mean density
  float rms;
  //! Number of labels being used
  int nlabl;
  //! text labels
  char labels[MRC_NUM_LABELS][MRC_LABEL_SIZE];
public:
  //! Converter from MRCHeader to DensityHeader
  void FromDensityHeader(const DensityHeader &h);
  //! Converter from DensityHeader to MRCHeader
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
  //! Empty constructor
  MRCReaderWriter(void){}
  //! Constructor
  /**
   * param[in] fn name of the file to open or write
   */
  MRCReaderWriter(char *fn) {
    filename=fn;
  }
  //! Reads an MRC file and translates the header to the general DensityHeader
  void Read(const char *fn_in, float **data, DensityHeader &head);
  //! Writes an MRC file from the data and the general DensityHeader
  void Write(const char *fn_out, const float *data, const DensityHeader &head);


private:


  //! By default the data are read into the grid of the class, but an external
  //! pointer to another grid can be specified
  void read(void) {
    read(&grid);
  }

  void read(float **pt);
  //! reads the header
  void read_header(void);
  //! reads the data
  void read_data(float *pt);
  //! reads data of size 8-bit
  void read_8_data(float *pt);
  //! reads data of size 32-bit
  void read_32_data(float *pt);
  void read_grid(void *pt,size_t size,size_t n);
  void seek_to_data(void);
  //! Write function
  /**
   * param[in] fn name of the file to write
   */
  void write(const char *fn) {
    return write(fn,grid);
  }

  //! Write function
  /**
   * param[in] fn name of the file to write
   * param[in] pt pointer to the data to write
   */
  void write(const char *fn,const float *pt);
  //! Writes the header
  /**
   * param[in] s stream to write the header
   */
  void write_header(std::ofstream &s);
  //! Writes data
  /**
   * param[in] s stream to write the data
   * param[in] pt pointer to the data
   */
  void write_data(std::ofstream &s, const float *pt);

  //! Name of the file
  std::string filename;
  //! file stream for the file read
  std::fstream fs;
  //! The header of the file
  MRCHeader header;
  //! The grid of data. The data is stored in the grid with the convention
  //! that the order of indexes is z,y,x
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
