#include "DensityMap.h"


DensityMap::DensityMap(){

  loc_calculated = false;
  normalized = false;
  rms_calculated = false;
  x_loc=NULL;y_loc=NULL;z_loc=NULL;
  data=NULL;
}



//TODO - update the copy cons
DensityMap::DensityMap(const DensityMap &other){

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
    if (data != NULL)
      delete data;
    if (x_loc != NULL)
      delete x_loc;
    if (y_loc != NULL)
      delete y_loc;
    if (z_loc != NULL)
      delete z_loc;

}

void DensityMap::CreateVoidMap(const int &nx,const int &ny,const int &nz) {
  int nvox = nx*ny*nz;
  data = new real[nvox];
  header.nx=nx;
  header.ny=ny;
  header.nz=nz;
}


// void DensityMap::Read(const string &filename, MapReaderWriter &reader) {
//   Read(filename.c_str(),reader);
// }

void DensityMap::Read(const char *filename, MapReaderWriter &reader) {
  //TODO: we need to decide who does the allocation ( mapreaderwriter or density)? if we keep the current implementation ( mapreaderwriter ) we need to pass a pointer to data
  if (reader.Read(filename,&data,header) != 0) {
    std::cerr << " DensityMap::Read unable to read map encoded in file : " << filename << std::endl;
    throw 1;
  }
  normalized = false;
  calcRMS();
  calc_all_voxel2loc();
}

void DensityMap::Write(const char *filename, MapReaderWriter &writer) {
  writer.Write(filename,data,header);
}

// void DensityMap::Write(const string &filename, MapReaderWriter &writer) {
//   Write(filename.c_str(),writer);
// }



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

int DensityMap::loc2voxel(float x,float y,float z) const
{
  if (!part_of_volume(x,y,z))
    throw std::out_of_range("the point is not part of the grid");
		      
  int ivoxx=(int)floor((x-header.get_xorigin())/header.Objectpixelsize);
  int ivoxy=(int)floor((y-header.get_yorigin())/header.Objectpixelsize);
  int ivoxz=(int)floor((z-header.get_zorigin())/header.Objectpixelsize);
  return ivoxz * header.nx * header.ny + ivoxy * header.nx + ivoxx;
}

bool DensityMap::part_of_volume(float x,float y,float z) const
{
  if( x>=header.get_xorigin() && x<=header.get_top(0) &&
      y>=header.get_yorigin() && y<=header.get_top(1) &&
      z>=header.get_zorigin() && z<=header.get_top(2) ) {
    return true;

  }
  else {
    return false;

  }
}

void DensityMap::calc_all_voxel2loc() {

  if (loc_calculated) 
    return;

  int nvox = header.nx*header.ny*header.nz;
  x_loc = new float[nvox];
  y_loc = new float[nvox];
  z_loc = new float[nvox];

  int ix=0,iy=0,iz=0;
  for (int ii=0;ii<nvox;ii++) {
    x_loc[ii] =  ix * header.Objectpixelsize + header.get_xorigin();
    y_loc[ii] =  iy * header.Objectpixelsize + header.get_yorigin();
    z_loc[ii] =  iz * header.Objectpixelsize + header.get_zorigin();

    // bookkeeping
    ix++;
    if (ix == header.nx) {
      ix = 0;
      ++iy;
      if (iy == header.ny) {
        iy = 0;
        ++iz;
      }
    }
  }
  loc_calculated = true;
}

void DensityMap::std_normalize() {
  
  if (normalized)
    return;
  
  float max_value=-1e40, min_value=1e40;
  float inv_std = 1.0/calcRMS();
  float mean = header.dmean;
  int nvox = header.nx * header.ny * header.nz;

  for (int ii=0;ii<nvox;ii++) {
    data[ii] = (data[ii] - mean) * inv_std;
    if(	data[ii]>max_value) max_value=data[ii];
    if(	data[ii]<min_value) min_value=data[ii];
  }
  normalized = true;
  rms_calculated=true;
  header.rms=1.;
  header.dmean=0.0;
  header.dmin=min_value;
  header.dmax=max_value;
}



float DensityMap::calcRMS() {

  if (rms_calculated) {
    return header.rms;
  }

  float max_value=-1e40, min_value=1e40;
  int  nvox = header.nx * header.ny * header.nz;
  float meanval = .0;
  float stdval = .0;

  for (int ii=0;ii<nvox;ii++) {
    meanval +=  data[ii];
    stdval += data[ii]*data[ii];
    if(	data[ii]>max_value) max_value=data[ii];
    if(	data[ii]<min_value) min_value=data[ii];

  }
  
  header.dmin=min_value;
  header.dmax=max_value;


  meanval /=  nvox;
  header.dmean = meanval;

  stdval = sqrt(stdval/nvox-meanval*meanval);
  header.rms = stdval;

  return stdval;
}

// data managment
void DensityMap::reset_data() {
  for (int i=0;i<header.nx*header.ny*header.nz;i++) {
    data[i]=0.0;
  }
  normalized = false;
  rms_calculated = false;
}



  void DensityMap::set_origin(float x,float y,float z) 
  {
    header.set_xorigin(x);header.set_yorigin(y);header.set_zorigin(z);
    header.compute_xyz_top(); // We have to compute the xmin,xmax, ... values again after changing the origin
    loc_calculated=false;
    delete(x_loc);delete(y_loc);delete(z_loc);
    calc_all_voxel2loc();
  }




bool DensityMap::same_origin  (const DensityMap &other) const{
    
  if( fabs(get_header()->get_xorigin()-other.get_header()->get_xorigin())<EPS &&
      fabs(get_header()->get_yorigin()-other.get_header()->get_yorigin())<EPS &&
      fabs(get_header()->get_zorigin()-other.get_header()->get_zorigin())<EPS)
    return true;
  return false;
    
  
}

bool DensityMap::same_dimensions  (const DensityMap &other) const {
  if(	get_header()->nx==other.get_header()->nx &&
	get_header()->ny==other.get_header()->ny &&
	get_header()->nz==other.get_header()->nz)
    return true;
  return false;
}

bool DensityMap::same_voxel_size  (const DensityMap &other) const{
  if( fabs(get_header()->Objectpixelsize-other.get_header()->Objectpixelsize)< EPS)
    return true;
  return false;
}

