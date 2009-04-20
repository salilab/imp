/**
 *  \file EMReaderWriter.cpp
 *  \brief Classes to read or write density files in EM format.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/EMReaderWriter.h>

IMPEM_BEGIN_NAMESPACE

EMHeader::EMHeader(const DensityHeader &header) {
  nx=header.nx;
  ny=header.ny;
  nz=header.nz;
  magic=header.magic;
  type=header.data_type;
  for (short i=0;i < DensityHeader::COMMENT_FIELD_SINGLE_SIZE; i++) {
    comment[i]=header.comments[0][i];
  }
  voltage=header.voltage;
  Cs=header.Cs;
  Aperture=header.Aperture;
  Magnification=header.Magnification;
  Postmagnification=header.Postmagnification;
  Exposuretime=header.Exposuretime;
  Objectpixelsize=header.Objectpixelsize;
  Microscope=header.Microscope;
  Pixelsize=header.Pixelsize;
  CCDArea=header.CCDArea;
  Defocus=header.Defocus;
  Astigmatism=header.Astigmatism;
  AstigmatismAngle=header.AstigmatismAngle;
  FocusIncrement=header.FocusIncrement;
  CountsPerElectron=header.CountsPerElectron;
  Intensity=header.Intensity;
  EnergySlitwidth=header.EnergySlitwidth;
  EnergyOffset=header.EnergyOffset;
  Tiltangle=header.Tiltangle;
  Tiltaxis=header.Tiltaxis;
  MarkerX=header.MarkerX;
  MarkerY=header.MarkerY;
  lswap=header.lswap;
}

void EMHeader::GenerateCommonHeader(DensityHeader &header) {
  header.nx=nx;
  header.ny=ny;
  header.nz=nz;
  header.magic=magic;
  header.data_type=type;

  for (short i=0;i < DensityHeader::COMMENT_FIELD_SINGLE_SIZE; i++) {
    header.comments[0][i]=comment[i];
  }

  header.voltage=voltage;
  header.Cs= Cs;
  header.Aperture=Aperture;
  header.Magnification=Magnification;
  header.Postmagnification= Postmagnification;
  header.Exposuretime=Exposuretime;
  header.Objectpixelsize=Objectpixelsize;
  header.Microscope=Microscope;
  header.Pixelsize=Pixelsize;
  header.CCDArea=CCDArea;
  header.Defocus=Defocus;
  header.Astigmatism=Astigmatism;
  header.AstigmatismAngle=AstigmatismAngle;
  header.FocusIncrement=FocusIncrement;
  header.CountsPerElectron=CountsPerElectron;
  header.Intensity=Intensity;
  header.EnergySlitwidth=EnergySlitwidth;
  header.EnergyOffset=EnergyOffset;
  header.Tiltangle=Tiltangle;
  header.Tiltaxis=Tiltaxis;
  header.MarkerX=MarkerX;
  MarkerY=header.MarkerY;
  header.lswap=lswap;
}


void EMReaderWriter::Read(const char *filename, float **data,
                         DensityHeader &header)
{
  std::ifstream file;
  file.open(filename, std::ifstream::in | std::ifstream::binary);
  if (!file.good()) {
    std::ostringstream msg;
    msg << " EMReaderWriter::Read >> The file "
    << filename << " was not found.\n";
    throw EMBED_IOException(msg.str().c_str());
  }
  file.exceptions(std::ifstream::eofbit | std::ifstream::failbit
                  | std::ifstream::badbit);
  EMHeader eheader;
  ReadHeader(file,eheader);

  if (eheader.Objectpixelsize < EPS) {
    eheader.Objectpixelsize = 1.;
    std::cout << "Objectpixelsize == 0 " << std::endl;
    std::cout << "  set Objectpixelsize = 1 to avoid trouble"<< std::endl;
  }
  eheader.GenerateCommonHeader(header);
  ReadData(file, data, eheader);
  file.close();
}



void EMReaderWriter::Write(const char* filename,const float *data,
                           const DensityHeader &header_)
{
  std::ofstream s(filename, std::ofstream::out | std::ofstream::binary);
  EMHeader header(header_);
  //init header data if not set
  if (header.type == 0) {
    header.type = 5;
  }
  WriteHeader(s,header);
  s.write((char *) data,sizeof(float)*header.nx*header.ny*header.nz);
  s.close();
}



/* swap bytes */
void swap(char *x, int size)
{
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



void EMReaderWriter::WriteHeader(std::ostream& s, const EMHeader &header)
{

  EMHeader::EMHeaderParse ehp;
  ehp.Init(header);

#ifdef IMP_LITTLE_ENDIAN
  ehp.emdata[EMHeader::EMHeaderParse::LSWAP_OFFSET] = 0;
#else
  ehp.emdata[EMHeader::EMHeaderParse::LSWAP_OFFSET] = 1;
  // byte-swap all ints in the header on big-endian machines:
  swap((char *)&ehp.nx, sizeof(int));
  swap((char *)&ehp.ny, sizeof(int));
  swap((char *)&ehp.nz, sizeof(int));
  for (int i = 0; i < 40; ++i) {
    swap((char *)&ehp.emdata[i], sizeof(int));
  }
#endif

  s.write((char *) &ehp,sizeof(EMHeader::EMHeaderParse));
  if(s.bad())
    {
      std::ostringstream msg;
      msg << " EMReaderWriter::WriteHeader >> Error writing header to file.\n";
      throw EMBED_IOException(msg.str().c_str());
    }
}




void EMReaderWriter::ReadHeader(std::ifstream &file, EMHeader &header)
{

  EMHeader::EMHeaderParse ehp;
  file.read((char *)&ehp, sizeof(EMHeader::EMHeaderParse));

#ifndef IMP_LITTLE_ENDIAN
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


void EMReaderWriter::ReadData(std::ifstream &file, float **data,
                             const EMHeader &header)
{

    int nvox = header.nx*header.ny*header.nz;

    // allocate data
    *data = new float[nvox];
    if (*data == NULL) {
      std::ostringstream msg;
      msg << " EMReaderWriter::ReadData >> can not allocated space for data. "
          "Requested size: " << nvox*sizeof(float) <<  "\n";
     throw EMBED_IOException(msg.str().c_str());
    }

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
      std::ostringstream msg;
      msg << "EMReaderWriter::ReadData the requested data type "
         << header.type << " is not implemented.\n";
     throw EMBED_IOException(msg.str().c_str());
    }

    char *voxeldata = new char[nvox * voxel_data_size];
    file.read(voxeldata,  voxel_data_size*nvox);
    char *tmp = new char[voxel_data_size];
#ifdef IMP_LITTLE_ENDIAN
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
