/**
 *  \file MRCReaderWriter.cpp
 *  \brief Classes to read or write MRC files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/endian.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/log.h>
#include <IMP/log_macros.h>
#include <boost/scoped_array.hpp>

IMPEM_BEGIN_NAMESPACE

void MRCReaderWriter::read(const char *fn_in, float **data,
                           DensityHeader &head) {
  // Read file
  filename.assign(fn_in);
  read(data);
  // Translate header to DensityHeader
  header.ToDensityHeader(head);
  head.Objectpixelsize_ = (float)head.xlen / head.get_nx();
}

void MRCReaderWriter::write(const char *fn_out, const float *data,
                            const DensityHeader &head) {
  // Translate DensityHeader to internal::MRCHeader and write
  header.FromDensityHeader(head);
  // Write
  write(fn_out, data);
}

void MRCReaderWriter::read(float **pt) {
  fs.open(filename.c_str(), std::fstream::in | std::fstream::binary);
  IMP_USAGE_CHECK(fs.good(), "The file " << filename << " was not found.");
  // Read header
  read_header();
  // Allocate memory
  size_t n = header.nc * header.nr * header.ns;
  (*pt) = new float[n];
  read_data(*pt);
  fs.close();

  // Convert data grid from col/row/segment to x/y/z if necessary
  if (header.mapc != 1 || header.mapr != 2 || header.maps != 3) {
    transpose_data_to_zyx(pt);
  }
}

void MRCReaderWriter::transpose_data_to_zyx(float **pt) {
  // Check for valid axis indices (should be a permutation of 1,2,3)
  if (header.mapc < 1 || header.mapc > 3
      || header.mapr < 1 || header.mapr > 3
      || header.maps < 1 || header.maps > 3
      || (header.mapc + header.mapr + header.maps != 6)) {
    IMP_THROW("Invalid axes in MRC file. Axes must be some "
              << "permutation of (1,2,3)", IOException);
  }

  // Get dimensions in xyz coordinate system
  int nx, ny, nz, nxstart, nystart, nzstart;
  header.convert_crs_to_xyz(nx, ny, nz, nxstart, nystart, nzstart);

  float *transposed = new float[nx * ny * nz];

  // Transpose data from pt[s,r,c] to transposed[z,y,x]
  int xyz[3] = {0,0,0}, csrind = 0;
  for (int s = 0; s < header.ns; ++s) {
    xyz[header.maps-1] = s;
    for (int r = 0; r < header.nr; ++r) {
      xyz[header.mapr-1] = r;
      for (int c = 0; c < header.nc; ++c, ++csrind) {
        xyz[header.mapc-1] = c;
        int xyzind = xyz[2] * ny * nx + xyz[1] * nx + xyz[0];
        transposed[xyzind] = (*pt)[csrind];
      }
    }
  }

  // Replace original data with transposed
  delete[] *pt;
  *pt = transposed;
}

void MRCReaderWriter::read_data(float *pt) {
  if (header.mode == 0) {
    return read_8_data(pt);
  } else if (header.mode == 2) {
    return read_32_data(pt);
  } else {
    IMP_THROW("MRCReaderWriter::read_data >> This routine can only read "
                  << "8-bit or 32-bit MRC files. Unknown mode for " << filename,
              IOException);
  }
}

/** Read the density data of a 8-bit MRC file */
void MRCReaderWriter::read_8_data(float *pt) {
  seek_to_data();
  size_t n = header.nc * header.ns * header.nr;
  boost::scoped_array<unsigned char> grid_8bit(new unsigned char[n]);
  read_grid(grid_8bit.get(), sizeof(unsigned char), n);
  // Transfer to floats
  for (size_t i = 0; i < n; i++) pt[i] = (float)grid_8bit[i];
  //  std::cout << "MRC file read in 8-bit mode: grid "
  //<< header.nx << "x" << header.ny << "x" << header.nz << "\n";
}

namespace {
/** Swaps the byte order in an array of 32-bit ints */
void byte_swap(unsigned char *ch, int n_array) {
  for (int c = n_array - 1; c >= 0; --c) {
    int i = c * 4;
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
void MRCReaderWriter::read_32_data(float *pt) {
  int needswap;
  seek_to_data();
  size_t n = header.nc * header.nr * header.ns;  // size of the grid
  read_grid(pt, sizeof(float), n);
  // Check for the necessity of changing the endian
  needswap = 0;
  for (size_t i = 0; i < n; i++)
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

void MRCReaderWriter::read_grid(void *pt, size_t size, size_t n) {
  fs.read((char *)pt, size * n);
  size_t val = fs.gcount();
  if (0) std::cout << val;
  IMP_USAGE_CHECK(val == size * n,
                  "MRCReaderWriter::read_grid >> The values read "
                  "are not the amount requested");
}

void MRCReaderWriter::seek_to_data() {
  fs.seekg(sizeof(internal::MRCHeader) + header.nsymbt, std::ios::beg);
  IMP_USAGE_CHECK(!fs.fail(),
                  "MRCReaderWriter::seek_to_data. Cannot find MRC data in file "
                      << filename);
}

void MRCReaderWriter::read_header() {
  // Read header
  fs.read((char *)&header, sizeof(internal::MRCHeader));
  IMP_USAGE_CHECK(
      fs.gcount() == sizeof(internal::MRCHeader),
      "MRCReaderWriter::read_header >> Error reading MRC header of file: "
          << filename);

  // Check for endian
  unsigned char *ch = (unsigned char *)&header;
  if ((ch[0] == 0 && ch[1] == 0) + algebra::get_is_big_endian() == 1) {
    int machinestamp = header.machinestamp;
    byte_swap(ch, 56);
    header.machinestamp = machinestamp;
  }
}

void MRCReaderWriter::write(const char *fn, const float *pt) {
  std::ofstream s(fn, std::ofstream::out | std::ofstream::binary);
  if (!s.fail()) {
    write_header(s);
    write_data(s, pt);
  }
  s.close();
}

void MRCReaderWriter::write_header(std::ofstream &s) {
  header.ispg = 1065353216;
  memcpy(header.map, "MAP ", 4);
  //  header.machinestamp = get_machine_stamp();
  int wordsize = 4;
  s.write((char *)&header.nc, wordsize);
  s.write((char *)&header.nr, wordsize);
  s.write((char *)&header.ns, wordsize);
  s.write((char *)&header.mode, wordsize);
  s.write((char *)&header.ncstart, wordsize);
  s.write((char *)&header.nrstart, wordsize);
  s.write((char *)&header.nsstart, wordsize);
  s.write((char *)&header.mx, wordsize);
  s.write((char *)&header.my, wordsize);
  s.write((char *)&header.mz, wordsize);
  s.write((char *)&header.xlen, wordsize);
  s.write((char *)&header.ylen, wordsize);
  s.write((char *)&header.zlen, wordsize);
  s.write((char *)&header.alpha, wordsize);
  s.write((char *)&header.beta, wordsize);
  s.write((char *)&header.gamma, wordsize);
  // Since we transpose from s/r/c to z/y/x on input, always output
  // 1,2,3 here so that we do not transpose the data again next time
  // we read it (we are using the xyz coordinate system, not crs)
  int mapc = 1, mapr = 2, maps = 3;
  s.write((char *)&mapc, wordsize);
  s.write((char *)&mapr, wordsize);
  s.write((char *)&maps, wordsize);
  s.write((char *)&header.dmin, wordsize);
  s.write((char *)&header.dmax, wordsize);
  s.write((char *)&header.dmean, wordsize);
  s.write((char *)&header.ispg, wordsize);
  s.write((char *)&header.nsymbt, wordsize);
  s.write((char *)&header.user, wordsize * IMP_MRC_USER);
  s.write((char *)&header.xorigin, wordsize);
  s.write((char *)&header.yorigin, wordsize);
  s.write((char *)&header.zorigin, wordsize);
  s.write((char *)&header.map, wordsize);
  s.write((char *)&header.machinestamp, wordsize);
  s.write((char *)&header.rms, wordsize);
  s.write((char *)&header.nlabl, wordsize);
  s.write((char *)&header.labels,
          sizeof(char) * IMP_MRC_NUM_LABELS * IMP_MRC_LABEL_SIZE);
  IMP_USAGE_CHECK(!s.bad(),
                  "MRCReaderWriter::write_header >> Error writing MRC header");
}

/* Writes the grid of values of an EM map to a MRC file */
void MRCReaderWriter::write_data(std::ofstream &s, const float *pt) {

  s.write((char *)pt, sizeof(float) * header.nc * header.nr * header.ns);
  IMP_USAGE_CHECK(!s.bad(),
                  "MRCReaderWriter::write_data >> Error writing MRC data.");
  IMP_LOG_TERSE("MRC file written: grid " << header.nc << "x" << header.nr
                                          << "x" << header.ns << std::endl);
}

IMPEM_END_NAMESPACE
