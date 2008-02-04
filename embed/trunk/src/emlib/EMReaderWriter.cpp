#include "EMReaderWriter.h"

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


int EMReaderWriter::Read(const char *filename, real **data, DensityHeader &header) {
  ifstream file;
  file.open(filename, ifstream::in | ifstream::binary);
  if (!file.good()) {
    cout << " the file " << filename << " was not found " << endl;
    return 1;
  }
  file.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
  EMHeader eheader;
  ReadHeader(file,eheader);
  if (eheader.Objectpixelsize < EPS) {
    eheader.Objectpixelsize = 1.;
    cout << "Objectpixelsize == 0 " << endl;
    cout << "  set Objectpixelsize = 1 to avoid trouble"<< endl;
  }

  eheader.GenerateCommonHeader(header);
  ReadData(file, data, eheader);
  file.close();
  return 0;
}



void EMReaderWriter::Write(const char* filename,const real *data, const DensityHeader &header_ ) {

  
  
  ofstream s(filename, ofstream::out | ofstream::binary);
  EMHeader header(header_);
  //init header data if not set
  if (header.type == 0) {
    header.type = 5;
  }
  WriteHeader(s,header);
  s.write((char *) data,sizeof(real)*header.nx*header.ny*header.nz);
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
 


int EMReaderWriter::WriteHeader(ostream& s, const EMHeader &header ) {
  
  EMHeader::EMHeaderParse ehp;
  ehp.Init(header);
  
#ifdef EM_LITTLE_ENDIAN
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
      cout << "EMReaderWriter::WriteHeader. Error writing header to file" << endl;
      return 1;
    }
  return 0;
}




int EMReaderWriter::ReadHeader(ifstream &file, EMHeader &header) {

  EMHeader::EMHeaderParse ehp;
  file.read((char *)&ehp, sizeof(EMHeader::EMHeaderParse));

#ifndef EM_LITTLE_ENDIAN
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
  return 0;
}


int EMReaderWriter::ReadData(ifstream &file, real **data, const EMHeader &header) {

    int nvox = header.nx*header.ny*header.nz;

    // allocate data
    *data = new real[nvox];
    if (*data == NULL) {
      cout << "EMReaderWriter::ReadData can not allocated space for data - the requested size: " << nvox*sizeof(real) << endl;
      return -1;
    }


    //a density of a single voxel can be reprented in 1 to 4 bytes. header.type provides this information.
    // 1 : byte 
    int voxel_data_size;
    if ( header.type == 1 ) {
      voxel_data_size =sizeof(unsigned char);
    }
    else if ( header.type == 2 ) {
	voxel_data_size =sizeof(int);
    }
    else if ( header.type == 5 ) {
	voxel_data_size =sizeof(float);
    }
    else {
      cout << "EMReaderWriter::ReadData the requested data type " << header.type << " is not implemented " << endl;
      return -1;
    }


    char *voxeldata = new char[nvox * voxel_data_size];
    file.read(voxeldata,  voxel_data_size*nvox);
    char *tmp = new char[voxel_data_size];
#ifdef EM_LITTLE_ENDIAN
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
    
    return 0;
}



