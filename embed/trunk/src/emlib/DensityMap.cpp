#include "DensityMap.h"


DensityMap::DensityMap(){
  loc_calculated = false;
  normalized = false;
  rms_calculated = false;
}



DensityMap::DensityMap(const DensityMap &other){
  cout << " DensityMap::DensityMap " << endl;
  header = other.header;
  int size = header.nx*header.ny*header.nz;
  data = new real[size];
  x_loc = new real[size];
  y_loc = new real[size];
  z_loc = new real[size];
  for (int i=0;i<header.nx*header.ny*header.nz;i++) {
    data[i] = other.data[i];
  }
  loc_calculated = other.loc_calculated;
  if (loc_calculated ) {
  for (int i=0;i<header.nx*header.ny*header.nz;i++) {
    x_loc[i] = other.x_loc[i];
    y_loc[i] = other.y_loc[i];
    z_loc[i] = other.z_loc[i];
  }
  }

  data_allocated = other.data_allocated;
  normalized = other.normalized;
  rms_calculated = other.rms_calculated;
}

DensityMap& DensityMap::operator=(const DensityMap& other) {
   if (this == &other) // protect against invalid self-assignment
     {
       return *this;
     }
   
   DensityMap *a = new DensityMap(other);
   return *a;
       
}



DensityMap::~DensityMap() {
  cout << " DensityMap::~DensityMap " << endl;
    delete data;
    delete x_loc;
    delete y_loc;
    delete z_loc;
  cout << " DensityMap::~DensityMap end" << endl;
  }




void DensityMap::CreateVoidMap(const int &nx,const int &ny,const int &nz) {
  int nvox = nx*ny*nz;
  data = new real[nvox];
  header.nx=nx;
  header.ny=ny;
  header.nz=nz;
}

void DensityMap::Read(const char *filename, MapReaderWriter &reader) {
  //TODO: we need to decide who does the allocation ( mapreaderwriter or density)? if we keep the current implementation ( mapreaderwriter ) we need to pass a pointer to data
  cout << " before read " << endl;
  reader.Read(filename,&data,header);
  cout << " after read " << endl;
  stdNormalize();
  cout << " after norm " << endl;
}
void DensityMap::Write(const char *filename, MapReaderWriter &writer) {
  writer.Write(filename,data,header);
}

float DensityMap::voxel2loc(const int &index,int dim) {
  if (!loc_calculated) 
    calc_all_voxel2loc();
  if (dim==0) {
    return x_loc[index];
    }
  else if (dim==1) {
    return y_loc[index];
    }
  //TODO - add error handling, dim is not 0,1,2
  return z_loc[index];
}

void DensityMap::calc_all_voxel2loc() {

  if (loc_calculated) 
    return;
  int nvox = header.nx*header.ny*header.nz;
  x_loc = new float[nvox];
  y_loc = new float[nvox];
  z_loc = new float[nvox];
  loc_calculated = true;



  int ix=0,iy=0,iz=0;

  for (int ii=0;ii<nvox;ii++) {
    x_loc[ii] =  ix * header.Objectpixelsize + header.xorigin;
    y_loc[ii] =  iy * header.Objectpixelsize + header.yorigin;
    z_loc[ii] =  iz * header.Objectpixelsize + header.zorigin;

    // bookkeeping
    ix = ix +1;
    if (ix == header.nx) {
      ix = 0;
      iy = iy+1;
      if (iy == header.ny) {
        iy = 0;
        iz = iz+1;
      }
    }
  }
}



void DensityMap::stdNormalize() {
  
  if (normalized)
    return;

  float inv_std = 1.0/calcRMS();
  float mean = header.dmean;
  int nvox = header.nx * header.ny * header.nz;
  for (int ii=0;ii<nvox;ii++) {
    data[ii] = (data[ii] - mean) * inv_std;
  }
  normalized = true;
  header.rms=1.;
  header.dmean=0.0;
}



float DensityMap::calcRMS() {

  if (rms_calculated)
    return header.rms;

  int  nvox = header.nx * header.ny * header.nz;
  float meanval = .0;
  float stdval = .0;

  for (int ii=0;ii<nvox;ii++) {
    meanval = meanval + data[ii];
    stdval = stdval + powf(data[ii], 2);
  }
  
  meanval /=  nvox;
  header.dmean = meanval;

  stdval = stdval - (powf(meanval, 2) * nvox);
  stdval = sqrt(stdval) / nvox;
  header.rms = stdval;

  return stdval;

}

