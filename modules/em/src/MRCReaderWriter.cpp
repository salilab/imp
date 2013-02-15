/**
 *  \file MRCReaderWriter.cpp
 *  \brief Classes to read or write MRC files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/endian.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

void MRCReaderWriter::read(const char *fn_in, float **data, DensityHeader &head)
{
  // Read file
  filename.assign(fn_in);
  read(data);
  // Translate header to DensityHeader
  header.ToDensityHeader(head);
  head.Objectpixelsize_ = (float)head.xlen/head.get_nx();
}

void MRCReaderWriter::write(const char *fn_out, const float *data,
                            const DensityHeader &head)
{
  // Translate DensityHeader to internal::MRCHeader and write
  header.FromDensityHeader(head);
  // Write
  write(fn_out,data);
}

void MRCReaderWriter::read(float **pt)
{
  fs.open(filename.c_str(), std::fstream::in | std::fstream::binary);
  IMP_USAGE_CHECK(fs.good(), "The file " << filename << " was not found.");
  // Read header
  read_header();
  // Allocate memory
  size_t n = header.nx*header.ny*header.nz;
  (*pt)= new float[n];
  read_data(*pt);
  fs.close();
}

void MRCReaderWriter::read_data(float *pt)
{
  if(header.mode==0) {
    return read_8_data(pt);
  }
  else if(header.mode==2) {
    return read_32_data(pt);
  }
  else {
    IMP_THROW("MRCReaderWriter::read_data >> This routine can only read "
              << "8-bit or 32-bit MRC files. Unknown mode for "
              << filename, IOException);
  }
}


/** Read the density data of a 8-bit MRC file */
void MRCReaderWriter::read_8_data(float *pt)
{
  seek_to_data();
  size_t n = header.nx*header.ny*header.nz;
  unsigned char *grid_8bit= new unsigned char [n]; // memory
  read_grid(grid_8bit,sizeof(unsigned char), n);
  // Transfer to floats
  for(size_t i=0;i<n;i++)
    pt[i]=(float)grid_8bit[i];
  delete(grid_8bit);
  //  std::cout << "MRC file read in 8-bit mode: grid "
  //<< header.nx << "x" << header.ny << "x" << header.nz << "\n";
}

namespace {
  /** Swaps the byte order in an array of 32-bit ints */
  void byte_swap(unsigned char *ch, int n_array)
  {
    for (int c = n_array -1; c >= 0; --c) {
      int i=c*4;
      unsigned char tmp = ch[i];
      ch[i] = ch[i + 3];
      ch[i + 3] = tmp;
      tmp = ch[i + 1];
      ch[i + 1] = ch[i + 2];
      ch[i + 2] = tmp;
    }
  }
}

/** Read the density data of a 32-bit MRC file */
void MRCReaderWriter::read_32_data(float *pt)
{
  int needswap;
  seek_to_data();
  size_t n = header.nx*header.ny*header.nz; // size of the grid
  read_grid(pt,sizeof(float), n);
  // Check for the necessity of changing the endian
  needswap = 0;
  for(size_t i=0;i<n;i++)
    if (pt[i] > 1e10 || pt[i] < -1e10) {
      // Really large values usually result if the endian is not correct
      needswap = 1;
      break;
    }
  /* Change endian if necessary */
  if (needswap == 1) {
    unsigned char *ch = (unsigned char *)pt;
    byte_swap(ch, n);
  }
  //  std::cout << "MRC file read in 32-bit mode: grid " << header.nx << "x"
  //          << header.ny << "x" << header.nz << std::endl;
}

void MRCReaderWriter::read_grid(void *pt,size_t size,size_t n)
{
  fs.read((char *)pt,size*n);
  size_t val = fs.gcount();
  if (0) std::cout << val;
  IMP_USAGE_CHECK(val == size*n,
            "MRCReaderWriter::read_grid >> The values read "
            "are not the amount requested");
}

void MRCReaderWriter::seek_to_data()
{
  fs.seekg(sizeof(internal::MRCHeader)+header.nsymbt, std::ios::beg);
  IMP_USAGE_CHECK(!fs.fail(),
            "MRCReaderWriter::seek_to_data. Cannot find MRC data in file "
            << filename);
}

void  MRCReaderWriter::read_header()
{
  // Read header
  fs.read((char *) &header,sizeof(internal::MRCHeader));
  IMP_USAGE_CHECK(fs.gcount() == sizeof(internal::MRCHeader),
            "MRCReaderWriter::read_header >> Error reading MRC header of file: "
            << filename);

  // Check for endian
  unsigned char *ch = (unsigned char *) &header;
  if ((ch[0] == 0 && ch[1] == 0) + algebra::get_is_big_endian() == 1) {
    int machinestamp = header.machinestamp;
    byte_swap(ch, 56);
    header.machinestamp = machinestamp;
  }
  IMP_USAGE_CHECK(header.mapc == 1 && header.mapr == 2 && header.maps == 3,
            "MRCReaderWriter::read_header >> Error reading MRC header of file: "
            << filename <<  "; Non-standard MRC file: column, row, section "
            << "indices are not (1,2,3) but (" << header.mapc << ","
            << header.mapr << "," << header.maps << ")."
            << " Resulting density data may be incorrectly oriented.");
}

void MRCReaderWriter::write(const char *fn,const float *pt)
{
  std::ofstream s(fn, std::ofstream::out | std::ofstream::binary);
  if(!s.fail()) {
    write_header(s);
    write_data(s,pt);
  }
  s.close();
}

void MRCReaderWriter::write_header(std::ofstream &s)
{
  header.ispg = 1065353216;
  memcpy(header.map, "MAP ", 4);
  //  header.machinestamp = get_machine_stamp();
  int wordsize=4;
  s.write((char *) &header.nx,wordsize);
  s.write((char *) &header.ny,wordsize);
  s.write((char *) &header.nz,wordsize);
  s.write((char *) &header.mode,wordsize);
  s.write((char *) &header.nxstart,wordsize);
  s.write((char *) &header.nystart,wordsize);
  s.write((char *) &header.nzstart,wordsize);
  s.write((char *) &header.mx,wordsize);
  s.write((char *) &header.my,wordsize);
  s.write((char *) &header.mz,wordsize);
  s.write((char *) &header.xlen,wordsize);
  s.write((char *) &header.ylen,wordsize);
  s.write((char *) &header.zlen,wordsize);
  s.write((char *) &header.alpha,wordsize);
  s.write((char *) &header.beta,wordsize);
  s.write((char *) &header.gamma,wordsize);
  s.write((char *) &header.mapc,wordsize);
  s.write((char *) &header.mapr,wordsize);
  s.write((char *) &header.maps,wordsize);
  s.write((char *) &header.dmin,wordsize);
  s.write((char *) &header.dmax,wordsize);
  s.write((char *) &header.dmean,wordsize);
  s.write((char *) &header.ispg,wordsize);
  s.write((char *) &header.nsymbt,wordsize);
  s.write((char *) &header.user,wordsize*IMP_MRC_USER);
  s.write((char *) &header.xorigin,wordsize);
  s.write((char *) &header.yorigin,wordsize);
  s.write((char *) &header.zorigin,wordsize);
  s.write((char *) &header.map,wordsize);
  s.write((char *) &header.machinestamp,wordsize);
  s.write((char *) &header.rms,wordsize);
  s.write((char *) &header.nlabl,wordsize);
  s.write((char *) &header.labels,
          sizeof(char)*IMP_MRC_NUM_LABELS*IMP_MRC_LABEL_SIZE);
  IMP_USAGE_CHECK(!s.bad(),
            "MRCReaderWriter::write_header >> Error writing MRC header");
}

/* Writes the grid of values of an EM map to a MRC file */
void MRCReaderWriter::write_data(std::ofstream &s,const float *pt)
{

  s.write((char *)pt,sizeof(float)*header.nx * header.ny * header.nz);
  IMP_USAGE_CHECK(!s.bad(),
            "MRCReaderWriter::write_data >> Error writing MRC data.");
  IMP_LOG_TERSE("MRC file written: grid " << header.nx << "x" << header.ny
          << "x" << header.nz << std::endl);
}

IMPEM_END_NAMESPACE
