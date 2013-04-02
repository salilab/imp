/**
 *  \file EMReaderWriter.cpp
 *  \brief Classes to read or write density files in EM format.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/EMReaderWriter.h>

IMPEM_BEGIN_NAMESPACE

void EMReaderWriter::read(const char *filename, float **data,
                         DensityHeader &header)
{
  std::ifstream file;
  file.open(filename, std::ifstream::in | std::ifstream::binary);
  IMP_USAGE_CHECK(file.good(),
            "EMReaderWriter::read >> The file " << filename
            << " was not found.");
  file.exceptions(std::ifstream::eofbit | std::ifstream::failbit
                  | std::ifstream::badbit);
  internal::EMHeader eheader;
  read_header(file,eheader);

  if (eheader.Objectpixelsize < EPS) {
    eheader.Objectpixelsize = 1.;
    //TODO - return the warning
    //    IMP_WARN("Objectpixelsize == 0 " << std::endl
   //<< "  set Objectpixelsize = 1 to avoid trouble"<< std::endl);
  }
  eheader.generate_common_header(header);
  read_data(file, data, eheader);
  file.close();
}



void EMReaderWriter::write(const char* filename,const float *data,
                           const DensityHeader &header_)
{
  std::ofstream s(filename, std::ofstream::out | std::ofstream::binary);
  internal::EMHeader header(header_);
  //init header data if not set
  if (header.type == 0) {
    header.type = 5;
  }
  write_header(s,header);
  s.write((char *) data,sizeof(float)*header.nx*header.ny*header.nz);
  s.close();
}


namespace {
  IMP_CLANG_PRAGMA(diagnostic push)
  IMP_CLANG_PRAGMA(diagnostic ignored "-Wcast-align")
/* swap bytes */
void swap(char *x, int size)
{
  IMP_INTERNAL_CHECK(reinterpret_cast<size_t>(x) % size == 0,
                     "Pointer is not properly aligned in swap");
  unsigned char c;
  unsigned short s;
  unsigned long l;

  switch (size)
  {
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
  IMP_CLANG_PRAGMA(diagnostic pop)
}


void EMReaderWriter::write_header(std::ostream& s,
                                  const internal::EMHeader &header)
{

  internal::EMHeader::EMHeaderParse ehp;
  ehp.Init(header);

#ifdef BOOST_LITTLE_ENDIAN
  ehp.emdata[internal::EMHeader::EMHeaderParse::LSWAP_OFFSET] = 0;
#else
  ehp.emdata[internal::EMHeader::EMHeaderParse::LSWAP_OFFSET] = 1;
  // byte-swap all ints in the header on big-endian machines:
  swap((char *)&ehp.nx, sizeof(int));
  swap((char *)&ehp.ny, sizeof(int));
  swap((char *)&ehp.nz, sizeof(int));
  for (int i = 0; i < 40; ++i) {
    swap((char *)&ehp.emdata[i], sizeof(int));
  }
#endif

  s.write((char *) &ehp,sizeof(internal::EMHeader::EMHeaderParse));
  IMP_USAGE_CHECK(!s.bad(),
            "EMReaderWriter::write_header >> Error writing header to file.");
}




void EMReaderWriter::read_header(std::ifstream &file,
                                 internal::EMHeader &header)
{

  internal::EMHeader::EMHeaderParse ehp;
  file.read((char *)&ehp, sizeof(internal::EMHeader::EMHeaderParse));

#ifndef BOOST_LITTLE_ENDIAN
  // byte-swap all ints in the header on big-endian machines:
  swap((char *)&ehp.nx, sizeof(int));
  swap((char *)&ehp.ny, sizeof(int));
  swap((char *)&ehp.nz, sizeof(int));
  for (int i = 0; i < 40; ++i) {
    swap((char *)&ehp.emdata[i], sizeof(int));
  }
#endif

  ehp.InitEMHeader(header);
  //header.Objectpixelsize = 1.0;
}


void EMReaderWriter::read_data(std::ifstream &file, float **data,
                               const internal::EMHeader &header)
{

    int nvox = header.nx*header.ny*header.nz;

    // allocate data
    *data = new float[nvox];
    IMP_USAGE_CHECK(*data,
              "EMReaderWriter::read_data >> can not allocated space for data. "
              "Requested size: " << nvox*sizeof(float));

    // a density of a single voxel can be reprented in 1 to 4 bytes.
    // header.type provides this information.
    // 1 : byte
    int voxel_data_size;
    switch(header.type) {
    case 1:
      voxel_data_size =sizeof(unsigned char);
      break;
    case 2:
      voxel_data_size =sizeof(int);
      break;
    case 5:
        voxel_data_size =sizeof(float);
      break;
    default:
      IMP_THROW("EMReaderWriter::read_data the requested data type "
                << header.type << " is not implemented.",
                IOException);
    }

    char *voxeldata = new char[nvox * voxel_data_size];
    file.read(voxeldata,  voxel_data_size*nvox);
    char *tmp = new char[voxel_data_size];
#ifdef BOOST_LITTLE_ENDIAN
    bool need_swap = (header.lswap == 1);
#else
    bool need_swap = (header.lswap != 1);
#endif
    for (int i=0;i<nvox;i++) {
      strncpy(tmp,&(voxeldata[i*voxel_data_size]),voxel_data_size);
      if (need_swap) {
        swap(tmp,voxel_data_size);
      }
      memcpy(&((*data)[i]),tmp,voxel_data_size);
    }
    delete[] tmp;
    delete[] voxeldata;
}

IMPEM_END_NAMESPACE
