#ifndef _MRCREADERWRITER_H
#define _MRCREADERWRITER_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  

  Javier Velazquez-Muriel

  OVERVIEW TEXT

*/


#include "MapReaderWriter.h"
#include <string.h>


#define MRC_LABEL_SIZE      80
#define MRC_USER            25
#define MRC_NUM_LABELS      10



/** MRC header */
class MRCHeader
{
	public:	
		int nz,ny,nx; // map size
		int mode; /* 0 	image : signed 8-bit bytes range -128 to 127. 1 image : 16-bit halfwords. 
					 2 	image : 32-bit reals. 3 	transform : complex 16-bit integers
					 4 	transform : complex 32-bit reals */
		int nxstart,nystart,nzstart; // number of first columns in map (default = 0)
		int mx, my, mz;  /* Number of intervals along each dimension */
		float xlen,ylen,zlen; // Cell dimensions (angstroms)
		float alpha, beta, gamma; // Cell angles (degrees)
		/** Axes corresponding to columns (mapc), rows (mapr) and sections (maps) (1,2,3 for x,y,z) */
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
		int FromDensityHeader(const DensityHeader &h);
		int ToDensityHeader(DensityHeader &h);
	
};


class MRCReaderWriter : public MapReaderWriter
{
	public:
		char *filename; // Name of the file
		fstream fs;  // file stream for the file read
		MRCHeader header; // The header of the file
		float *grid;     // The grid of data. The data is stored in the grid with the convention that the order of indexes is z,y,x
						   // the grid index is n = nx*ny*k+ny*j+i
	public:

		MRCReaderWriter(void){}
		MRCReaderWriter(char *fn) {
			filename=fn;
		}

		int Read(const char *fn_in, real **data, DensityHeader &head);
		void Write(const char *fn_out, const real *data, const DensityHeader &head);

	
		// By default the data are read into the grid of the class, but an external pointer to another grid can be especified
		int read(void) {
		  return read(&grid);
		}
		int read(float **pt);

		// read the header
		int read_header(void);
		// read different modes
		int read_data(float *pt);
		int read_8_data(float *pt);
		int read_32_data(float *pt);
		int read_grid(void *pt,size_t size,size_t n);
		int seek_to_data(void);
		// Write functions
		int write(char *fn)
		{
			return write(fn,grid);
		}	
		int write(char *fn,const float *pt);
		int write_header(fstream *f_out);
		int write_data(fstream *f_out,const float *pt);
};



int get_machine_stamp(void);
int is_bigendian(void);
void byte_swap(unsigned char *ch, int n_array);


#endif //_MRCREADERWRITER_H
