/**
 *  \file DensityMap.cpp
 *  \brief Class for handling density maps.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/DensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/XplorReaderWriter.h>
#include <IMP/em/EMReaderWriter.h>
#include <IMP/Pointer.h>
#include <boost/algorithm/string/predicate.hpp>
#include <climits>

IMPEM_BEGIN_NAMESPACE
namespace {
  template <class T>
   bool is_initialize(const T &t) {
     if (std::numeric_limits<T>::has_signaling_NaN) {
       return (t == std::numeric_limits<T>::signaling_NaN());
     } else if (std::numeric_limits<T>::has_quiet_NaN) {
       return (t == std::numeric_limits<T>::quiet_NaN());
     } else if (std::numeric_limits<T>::has_infinity) {
       return (t== std::numeric_limits<T>::infinity());
     } else {
       // numerical limits for int and double have completely
       // different meanings of max/min
       return (t == -std::numeric_limits<T>::max());
     }
  }
}

DensityMap::DensityMap()
{
  loc_calculated_ = false;
  normalized_ = false;
  rms_calculated_ = false;
}



//TODO - update the copy cons
DensityMap::DensityMap(const DensityMap &other)
{
  header_ = other.header_;
  long size = get_number_of_voxels();
  data_.reset(new emreal[size]);
  std::copy(other.data_.get(), other.data_.get()+size, data_.get());
  loc_calculated_ = other.loc_calculated_;
  if (loc_calculated_) {
    x_loc_.reset(new float[size]);
    y_loc_.reset(new float[size]);
    z_loc_.reset(new float[size]);
    std::copy(other.x_loc_.get(), other.x_loc_.get()+size, x_loc_.get());
    std::copy(other.y_loc_.get(), other.y_loc_.get()+size, y_loc_.get());
    std::copy(other.z_loc_.get(), other.z_loc_.get()+size, z_loc_.get());
  } else {
    x_loc_.reset();
    y_loc_.reset();
    z_loc_.reset();
  }

  data_allocated_ = other.data_allocated_;
  normalized_ = other.normalized_;
  rms_calculated_ = other.rms_calculated_;
}

DensityMap& DensityMap::operator=(const DensityMap& other)
{
  if (this == &other) { // protect against invalid self-assignment
    return *this;
  }

  DensityMap *a = new DensityMap(other);
  return *a;
}



void DensityMap::CreateVoidMap(const int &nx, const int &ny, const int &nz)
{
  long nvox = nx*ny*nz;
  data_.reset(new emreal[nvox]);
  for (long i=0;i<nvox;i++) {
    data_[i]=0.0;
  }
  header_.set_number_of_voxels(nx,ny,nz);
}

#ifndef IMP_NO_DEPRECATED
void DensityMap::Read(const char *filename, MapReaderWriter &reader) {
  // TODO: we need to decide who does the allocation ( mapreaderwriter or
  // density)? if we keep the current implementation ( mapreaderwriter )
  // we need to pass a pointer to data_
  float *f_data;
  reader.Read(filename, &f_data, header_);
  boost::scoped_array<float> fp_data(f_data);
  float2real(f_data, data_);
  normalized_ = false;
  calcRMS();
  calc_all_voxel2loc();
  header_.compute_xyz_top();
  if (header_.get_spacing() == 1.0) {
    IMP_WARN("The pixel size is set to the default value 1.0."<<
              "Please make sure that this is indeed the pixel size of the map");
  }
}
#endif

DensityMap* read_map(const char *filename) {
  std::string name(filename);
  if (boost::algorithm::ends_with(name, std::string(".mrc"))) {
    MRCReaderWriter rw;
    return read_map(filename, rw);
  } else if (boost::algorithm::ends_with(name, std::string(".em"))) {
    EMReaderWriter rw;
    return read_map(filename, rw);
  } else if (boost::algorithm::ends_with(name, std::string(".xplor"))) {
    XplorReaderWriter rw;
    return read_map(filename, rw);
  } else {
    IMP_THROW("Unable to determine type for file "<< filename,
              IOException);
  }
}

void DensityMap::update_header() {
  if (not is_initialize(header_.get_nx())) {
  }
  if (not is_initialize(header_.get_ny())) {
  }
  if (not is_initialize(header_.get_nz())) {
  }
  IMP_INTERNAL_CHECK(is_initialize(header_.get_data_type()),
      "The data_type of the map header is not initialized");
  if (not is_initialize(header_.nxstart)){
  }
  if (not is_initialize(header_.nystart)){
  }
  if (not is_initialize(header_.nzstart)){
  }
  if (not is_initialize(header_.mx)){
  }
  if (not is_initialize(header_.my)){
  }
  if (not is_initialize(header_.mz)){
  }
  if (not is_initialize(header_.xlen)){
  }
  if (not is_initialize(header_.ylen)){
  }
  if (not is_initialize(header_.zlen)){
  }
  if (not is_initialize(header_.alpha)){
  }
  if (not is_initialize(header_.beta)){
  }
  if (not is_initialize(header_.gamma)){
  }
  if (not is_initialize(header_.mapc)){
  }
  if (not is_initialize(header_.mapr)){
  }
  if (not is_initialize(header_.maps)){
  }
  if (not is_initialize(header_.dmin)){
    header_.dmin = get_min_value();
  }
  if (not is_initialize(header_.dmax)){
    header_.dmax = get_max_value();
  }
  /*initialize(dmean);
   initialize(ispg);
   initialize(nsymbt);
   initialize(machinestamp);
   initialize(rms);
   initialize(nlabl);
   initialize(magic);
   initialize(voltage);
   initialize(Cs);  //Cs of microscope
   initialize(Aperture);  //Aperture used
   initialize(Magnification);  //Magnification
   initialize(Postmagnification); //Postmagnification (of energy filter)
   initialize(Exposuretime); //Exposuretime
   initialize(Microscope);  //Microscope
   initialize(Pixelsize); //Pixelsize - used for the microscope CCD camera
   initialize(CCDArea);  //CCDArea
   initialize(Defocus);  //Defocus
   initialize(Astigmatism);//Astigmatism
   initialize(AstigmatismAngle); //Astigmatism Angle
   initialize(FocusIncrement);//Focus-Increment
   initialize(CountsPerElectron);//Counts/Electron
   initialize(Intensity);//Intensity
   initialize(EnergySlitwidth);//Energy slitwidth of energy filter
   initialize(EnergyOffset); //Energy offset of Energy filter
   initialize(Tiltangle);//Tiltangle of stage
   initialize(Tiltaxis);//Tiltaxis
   initialize(MarkerX);//Marker_X coordinate
   initialize(MarkerY);//Marker_Y coordinate
   initialize(lswap);
  */
  IMP_INTERNAL_CHECK(is_initialize(header_.Objectpixelsize_),
                     "The map voxel size is not initialized"<<std::endl);
  IMP_INTERNAL_CHECK(is_initialize(header_.get_top(0)) &&
                     is_initialize(header_.get_top(1)) &&
                     is_initialize(header_.get_top(2)),
                     "The top value of the header is not initalized"
                     <<std::endl);
  IMP_INTERNAL_CHECK(is_initialize(header_.get_xorigin()) &&
                     is_initialize(header_.get_xorigin()) &&
                     is_initialize(header_.get_xorigin()),
                     "The origin value of the header is not initalized"
                     <<std::endl);
  IMP_INTERNAL_CHECK(is_initialize(header_.get_resolution()),
                           "The resolution was not initialized"<<std::endl);
}

DensityMap* read_map(const char *filename, MapReaderWriter &reader)
{
  // TODO: we need to decide who does the allocation ( mapreaderwriter or
  // density)? if we keep the current implementation ( mapreaderwriter )
  // we need to pass a pointer to data_
  Pointer<DensityMap> m= new DensityMap();
  float *f_data=NULL;
  reader.Read(filename, &f_data, m->header_);
  boost::scoped_array<float> f_datap(f_data);
  m->float2real(f_datap.get(), m->data_);
  m->normalized_ = false;
  m->calcRMS();
  m->calc_all_voxel2loc();
  m->header_.compute_xyz_top();
  if (m->header_.get_spacing() == 1.0) {
    IMP_WARN("The pixel size is set to the default value 1.0."<<
              "Please make sure that this is indeed the pixel size of the map"
             << std::endl);
  }
  m->set_name(filename);
  IMP_LOG(TERSE, "Read range is "
          << *std::max_element(m->data_.get(),
                               m->data_.get()+m->get_number_of_voxels())
          << "..." << *std::min_element(m->data_.get(),
                                        m->data_.get()
                                        +m->get_number_of_voxels())
          << std::endl);
  return m.release();
}

void DensityMap::float2real(float *f_data,
                            boost::scoped_array<emreal> &r_data)
{
  long size = get_number_of_voxels();
  r_data.reset(new emreal[size]);
  // let the compiler optimize it
  std::copy(f_data, f_data+size, r_data.get());
}

void DensityMap::real2float(emreal *r_data,
                            boost::scoped_array<float> &f_data)
{
  long size = get_number_of_voxels();
  f_data.reset(new float[size]);
  std::copy(r_data, r_data+size, f_data.get());
}

#ifndef IMP_DEPRECATED

void DensityMap::Write(const char *filename, MapReaderWriter &writer) {
  IMP::em::write_map(this, filename, writer);
}
#endif

void write_map(DensityMap *d, const char *filename, MapReaderWriter &writer)
{
  boost::scoped_array<float> f_data;
  d->real2float(d->data_.get(), f_data);
  writer.Write(filename, f_data.get(), d->header_);
}

long DensityMap::get_number_of_voxels() const {
  return header_.get_number_of_voxels();
}

float DensityMap::voxel2loc(const int &index, int dim) const
{
  IMP_USAGE_CHECK(loc_calculated_,
            "locations should be calculated prior to calling this function");
//   if (!loc_calculated_)
//     calc_all_voxel2loc();
  IMP_USAGE_CHECK(dim >= 0 && dim <= 2,
            "the dim index should be 0-2 (x-z) dim value:" << dim);
  if (dim==0) {
    return x_loc_[index];
    }
  else if (dim==1) {
    return y_loc_[index];
    }
  return z_loc_[index];
}

long DensityMap::xyz_ind2voxel(int voxx,int voxy,int voxz) const{
  return voxz * header_.get_nx() * header_.get_ny() +
         voxy * header_.get_nx() + voxx;
}

long DensityMap::loc2voxel(float x,float y,float z) const
{
  IMP_USAGE_CHECK(is_part_of_volume(x,y,z),
            "The point is not part of the grid");
  int ivoxx=static_cast<int>(std::floor((x-header_.get_xorigin())
                                        /header_.get_spacing()));
  int ivoxy=static_cast<int>(std::floor((y-header_.get_yorigin())
                                        /header_.get_spacing()));
  int ivoxz=static_cast<int>(std::floor((z-header_.get_zorigin())
                                        /header_.get_spacing()));
  return xyz_ind2voxel(ivoxx,ivoxy,ivoxz);
}

bool DensityMap::is_xyz_ind_part_of_volume(int ix,int iy,int iz) const
{
  if( ix>=0 && ix<header_.get_nx() &&
      iy>=0 && iy<header_.get_ny() &&
      iz>=0 && iz<header_.get_nz() )
    return true;
  return false;
}


bool DensityMap::is_part_of_volume(float x,float y,float z) const
{
  if( x>=header_.get_xorigin() && x<=header_.get_top(0) &&
      y>=header_.get_yorigin() && y<=header_.get_top(1) &&
      z>=header_.get_zorigin() && z<=header_.get_top(2) ) {
    return true;

  }
  else {
    return false;

  }
}

emreal DensityMap::get_value(float x, float y, float z) const {
  return data_[loc2voxel(x,y,z)];
}

emreal DensityMap::get_value(long index) const {
  IMP_USAGE_CHECK(index >= 0 && index < get_number_of_voxels(),
            "The index " << index << " is not part of the grid");
  return data_[index];
}

void DensityMap::set_value(long index,emreal value) {
  IMP_USAGE_CHECK(index >= 0 && index < get_number_of_voxels(),
            "The index " << index << " is not part of the grid");
  data_[index]=value;
  normalized_ = false;
  rms_calculated_ = false;
}

void DensityMap::set_value(float x, float y, float z,emreal value) {
  data_[loc2voxel(x,y,z)]=value;
  normalized_ = false;
  rms_calculated_ = false;
}


void DensityMap::reset_voxel2loc() {
  loc_calculated_=false;
  x_loc_.reset();
  y_loc_.reset();
  z_loc_.reset();
}
void DensityMap::calc_all_voxel2loc()
{
  if (loc_calculated_)
    return;

  long nvox = get_number_of_voxels();
  x_loc_.reset(new float[nvox]);
  y_loc_.reset(new float[nvox]);
  z_loc_.reset(new float[nvox]);

  int ix=0,iy=0,iz=0;
  float hspace= header_.get_spacing() /2.0;
  for (long ii=0;ii<nvox;ii++) {
    x_loc_[ii] =  ix * header_.get_spacing() + header_.get_xorigin()+hspace;
    y_loc_[ii] =  iy * header_.get_spacing() + header_.get_yorigin()+hspace;
    z_loc_[ii] =  iz * header_.get_spacing() + header_.get_zorigin()+hspace;

    // bookkeeping
    ix++;
    if (ix == header_.get_nx()) {
      ix = 0;
      ++iy;
      if (iy == header_.get_ny()) {
        iy = 0;
        ++iz;
      }
    }
  }
  loc_calculated_ = true;
}

void DensityMap::std_normalize()
{
  if (normalized_)
    return;

  float max_value=-1e40, min_value=1e40;
  float inv_std = 1.0/calcRMS();
  float mean = header_.dmean;
  long nvox = get_number_of_voxels();

  for (long ii=0;ii<nvox;ii++) {
    data_[ii] = (data_[ii] - mean) * inv_std;
    if(data_[ii] > max_value) max_value = data_[ii];
    if(data_[ii] < min_value) min_value = data_[ii];
  }
  normalized_ = true;
  rms_calculated_ = true;
  header_.rms = 1.;
  header_.dmean = 0.0;
  header_.dmin = min_value;
  header_.dmax = max_value;
}



emreal DensityMap::calcRMS()
{

  if (rms_calculated_) {
    return header_.rms;
  }

  emreal max_value=-1e40, min_value=1e40;
  long  nvox = get_number_of_voxels();
  emreal meanval = .0;
  emreal stdval = .0;

  for (long ii=0;ii<nvox;ii++) {
    meanval += data_[ii];
    stdval += data_[ii] * data_[ii];
    if(data_[ii] > max_value) max_value = data_[ii];
    if(data_[ii] < min_value) min_value = data_[ii];

  }

  header_.dmin=min_value;
  header_.dmax=max_value;


  meanval /=  nvox;
  header_.dmean = meanval;

  stdval = sqrt(stdval/nvox-meanval*meanval);
  header_.rms = stdval;

  return stdval;
}

//!  Set the density voxels to zero and reset the managment flags.
void DensityMap::reset_data(float val)
{
  for (long i = 0; i < get_number_of_voxels(); i++) {
    data_[i] = val;
  }
  normalized_ = false;
  rms_calculated_ = false;
}



void DensityMap::set_origin(float x, float y, float z)
{
  header_.set_xorigin(x); header_.set_yorigin(y); header_.set_zorigin(z);
  // We have to compute the xmin,xmax, ... values again after
  // changing the origin
  header_.compute_xyz_top();
  reset_voxel2loc();
  calc_all_voxel2loc();
}




bool DensityMap::same_origin(const DensityMap &other) const
{
  if( fabs(get_header()->get_xorigin()-other.get_header()->get_xorigin())<EPS &&
      fabs(get_header()->get_yorigin()-other.get_header()->get_yorigin())<EPS &&
      fabs(get_header()->get_zorigin()-other.get_header()->get_zorigin())<EPS)
    return true;
  return false;
}

bool DensityMap::same_dimensions(const DensityMap &other) const
{
  if (get_header()->get_nx()==other.get_header()->get_nx() &&
      get_header()->get_ny()==other.get_header()->get_ny() &&
      get_header()->get_nz()==other.get_header()->get_nz())
    return true;
  return false;
}

bool DensityMap::same_voxel_size(const DensityMap &other) const
{
  if(fabs(get_header()->get_spacing()
          - other.get_header()->get_spacing()) < EPS)
    return true;
  return false;
}

algebra::VectorD<3> DensityMap::get_centroid(emreal threshold)  const{
  IMP_CHECK_CODE(emreal max_val = get_max_value());
  IMP_USAGE_CHECK(threshold < max_val,
            "The input threshold with value " << threshold
            << " is higher than the maximum density in the map " << max_val);
  float x_centroid = 0.0;
  float y_centroid = 0.0;
  float z_centroid = 0.0;
  int counter = 0;
  long nvox = get_number_of_voxels();
  for (long i=0;i<nvox;i++) {
    if (data_[i] <= threshold) {
      continue;
    }
    x_centroid += voxel2loc(i,0);
    y_centroid += voxel2loc(i,1);
    z_centroid += voxel2loc(i,2);
    counter += 1;
  }
  //counter will not be 0 since we checked that threshold is within
  //the map densities.
  x_centroid /= counter;
  y_centroid /= counter;
  z_centroid /= counter;
  return algebra::VectorD<3>(x_centroid,y_centroid,z_centroid);
}
emreal DensityMap::get_max_value() const{
  emreal max_val = -1.0 * INT_MAX;
  long nvox = get_number_of_voxels();
  for (long i=0;i<nvox;i++) {
    if (data_[i] > max_val) {
      max_val = data_[i];
    }
  }
  return max_val;
}

emreal DensityMap::get_min_value() const{
  emreal min_val = INT_MAX;
  long nvox = get_number_of_voxels();
  for (long i=0;i<nvox;i++) {
    if (data_[i] < min_val) {
      min_val = data_[i];
    }
  }
  return min_val;
}

std::string DensityMap::get_locations_string(float t)  {
  std::stringstream out;
  long nvox = get_number_of_voxels();
  float x,y,z;
  for (long i=0;i<nvox;i++) {
    if (data_[i] > t) {
      x=voxel2loc(i,0);
      y=voxel2loc(i,1);
      z=voxel2loc(i,2);
      out<<x << " " << y << " " << z << std::endl;
    }
  }
  return out.str();
}

void DensityMap::multiply(float factor) {
  long size = header_.get_number_of_voxels();
  for (long i=0;i<size;i++){
    data_[i]= factor*data_[i];
  }
}

void DensityMap::add(const DensityMap &other) {
  //check that the two maps have the same dimensions
  IMP_USAGE_CHECK(same_dimensions(other),
    "The dimensions of the two maps differ ( " << header_.get_nx()
     << "," << header_.get_ny() << "," << header_.get_nz() << ") != ("
     << other.header_.get_nx() << "," << other.header_.get_ny() << ","
     << other.header_.get_nz() << " ) ");
  // check that the two maps have the same voxel size
  IMP_USAGE_CHECK(same_voxel_size(other),
            "The voxel sizes of the two maps differ ( "
            << header_.get_spacing() << " != "
            << other.header_.get_spacing());
  long size = header_.get_number_of_voxels();
  for (long i=0;i<size;i++){
    data_[i]= data_[i] + other.data_[i];
  }
}


void DensityMap::pad(int nx, int ny, int nz,float val) {

  IMP_USAGE_CHECK(nx >= header_.get_nx() &&
                  ny >= header_.get_ny() &&
                  nz >= header_.get_nz(),
            "The requested volume is smaller than the existing one");

  long new_size = nx*ny*nz;
  long cur_size = get_number_of_voxels();
  reset_voxel2loc();
  calc_all_voxel2loc();
  boost::scoped_array<emreal> new_data(new emreal[new_size]);
  for (long i = 0; i < new_size; i++) {
    new_data[i] = val;
  }
  int new_vox_x,new_vox_y,new_vox_z;
  long new_vox;
  for (long i = 0; i <  cur_size; i++) {
    float x = voxel2loc(i,0);
    float y = voxel2loc(i,1);
    float z = voxel2loc(i,2);
    new_vox_x=(int)floor((x-header_.get_xorigin())/header_.get_spacing());
    new_vox_y=(int)floor((y-header_.get_yorigin())/header_.get_spacing());
    new_vox_z=(int)floor((z-header_.get_zorigin())/header_.get_spacing());
    new_vox =  new_vox_z * nx * ny + new_vox_y * nx + new_vox_x;
    new_data[new_vox] = data_[i];
  }
  header_.update_map_dimensions(nx,ny,nz);
  data_.swap(new_data);
  calc_all_voxel2loc();
}


void DensityMap::update_voxel_size(float new_apix) {
  header_.Objectpixelsize_=new_apix;
  header_.update_cell_dimensions();
  header_.compute_xyz_top(true);
  reset_voxel2loc();
  calc_all_voxel2loc();
}


Float approximate_molecular_mass(DensityMap* d, Float threshold) {
  long counter=0;//number of voxles above the threshold
  for(long l=0;l<d->get_number_of_voxels();l++) {
    if (d->get_value(l) > threshold) {
      ++counter;
     }
   }
  return d->get_spacing()*counter/1.21;
 }



/* Daniel's helpers */
namespace {
  inline algebra::VectorD<3> get_voxel_center(const DensityMap *map,
                                     unsigned int v) {
    return algebra::VectorD<3>(map->voxel2loc(v,0),
                             map->voxel2loc(v,1),
                             map->voxel2loc(v,2));
  }

  DensityMap *create_density_map(const algebra::BoundingBox3D &bb,
                                 double spacing) {
    Pointer<DensityMap> ret(new DensityMap());
    unsigned int n[3];
    algebra::VectorD<3> wid= bb.get_corner(1)-bb.get_corner(0);
    for (unsigned int i=0; i< 3; ++i) {
      n[i]= static_cast<int>(std::ceil(wid[i]/spacing));
    }
    ret->CreateVoidMap(n[0], n[1], n[2]);
    ret->set_origin(bb.get_corner(0));
    ret->update_voxel_size(spacing);
    IMP_LOG(TERSE, "Created map with dimensions " << n[0] << " " << n[1]
            << " " << n[2] << " and spacing " << ret->get_spacing()
            << std::endl);
    return ret.release();
  }

  /* Surround the density map with an extra set of samples assumed to
     be 0.
  */
  inline double get_value(const DensityMap *m, int xi,
                          int yi, int zi) {
    //std::cout << "getting " << xi << ' ' << yi << ' ' << zi << std::endl;
    if (xi < 0 || yi < 0 || zi < 0) return 0.0;
    else if (xi >= m->get_header()->get_nx()
             || yi >= m->get_header()->get_ny()
             || zi >= m->get_header()->get_nz()) return 0.0;
    else {
      unsigned int loc= m->xyz_ind2voxel(xi, yi, zi);
      //std::cout << "got " << m->get_value(loc) << std::endl;
      double v= m->get_value(loc);
      //std::cout << v << " " << " for " << loc << std::endl;
      return v;
    }
  }

  inline void compute_voxel(const DensityMap *m, const algebra::VectorD<3> &v,
                            int *ivox, algebra::VectorD<3> &remainder) {
    const double iside= 1.0/m->get_spacing();
    //std::cout << "getting " << v << std::endl;
    for (unsigned int i=0; i< 3; ++i) {
      double fvox= (v[i]- m->get_origin()[i])*iside;
      ivox[i]= static_cast<int>(std::floor(fvox));
    //std::cout << "setting ivox " << i << " to "
      // << ivox[i] << " for " << fvox << std::endl;
      remainder[i]= fvox-ivox[i];
      IMP_INTERNAL_CHECK(remainder[i] < 1.01 && remainder[i] >= -.01,
                         "Algebraic error " << remainder[i]
                         << " " << i << v << get_bounding_box(m)
                         << " " << m->get_spacing()
                         << " " << m->get_origin()
                         << " " << fvox);
    }
  }
  inline unsigned int get_n(const DensityMap *m, unsigned int dim) {
    switch (dim) {
    case 0:
      return m->get_header()->get_nx();
    case 1:
      return m->get_header()->get_ny();
    default:
      return m->get_header()->get_nz();
    }
  }
}


algebra::BoundingBox3D get_bounding_box(DensityMap* d,Float threshold) {
  algebra::BoundingBox3D ret;
  for(long l=0;l<d->get_number_of_voxels();l++) {
    if (d->get_value(l) > threshold) {
      algebra::VectorD<3> v(get_voxel_center(d, l));
      ret+= v;
     }
  }
  // make sure it includes the whole voxels
  return ret+=d->get_spacing()/2.0;
}


double get_density(DensityMap *m, const algebra::VectorD<3> &v) {
  // trilirp in z, y, x
  const double side= m->get_spacing();
  const double halfside= side/2.0;
  for (unsigned int i=0; i< 3; ++i){
    if (v[i] < m->get_origin()[i]-halfside
        || v[i] >= m->get_top()[i]+halfside) {
      //std::cout << v << " was rejected." << std::endl;
      return 0;
    }
  }

  int ivox[3];
  algebra::VectorD<3> r;
  compute_voxel(m, v, ivox, r);
  double is[4];
  for (unsigned int i=0; i< 4; ++i) {
    // operator >> has high precidence compared. Go fig.
    unsigned int bx= ((i&2) >> 1);
    unsigned int by= (i&1);
    assert((bx==0 || bx==1) && (by==0 || by==1));
    is[i]= get_value(m, ivox[0]+bx, ivox[1]+by, ivox[2])*(1-r[2])
      + get_value(m, ivox[0]+bx, ivox[1]+by, ivox[2]+1U)*(r[2]);
  }

  double js[2];
  for (unsigned int i=0; i< 2; ++i) {
    js[i]= is[i*2] * (1- r[1]) + is[i*2+1] * (r[1]);
  }
  return js[0]*(1-r[0]) + js[1]*(r[0]);
}

DensityMap *get_transformed_internal(DensityMap *in,
          const algebra::Transformation3D &tr,
          const algebra::BoundingBox3D &nbb){
  IMP::Pointer<DensityMap> ret(create_density_map(nbb,
                                 in->get_header()->get_spacing()));
  const algebra::Transformation3D &tri= tr.get_inverse();
  unsigned int size=ret->get_number_of_voxels();
  for (unsigned int i=0; i< size; ++i) {
    algebra::VectorD<3> tpt=get_voxel_center(ret, i);
    algebra::VectorD<3> pt= tri.get_transformed(tpt);
    double d = get_density(in, pt);
    ret->set_value(i, d);
  }
  ret->set_name(std::string("transformed ")+ in->get_name());
  if (in->get_header()->get_has_resolution()) {
    ret->get_header_writable()
      ->set_resolution(in->get_header()->get_resolution());
  }
  return ret.release();
}
DensityMap *get_transformed(DensityMap *in, const algebra::Transformation3D &tr,
                            double threshold) {
  algebra::BoundingBox3D obb= get_bounding_box(in, threshold);
  return get_transformed_internal(in,tr,obb);
}



DensityMap *get_transformed(DensityMap *in,
                           const algebra::Transformation3D &tr){
  algebra::BoundingBox3D obb= get_bounding_box(in);
  return get_transformed_internal(in,tr,obb);
}

DensityMap* get_resampled(DensityMap *in, double scaling) {
  algebra::BoundingBox3D obb= get_bounding_box(in,
                                         -std::numeric_limits<float>::max());
  Pointer<DensityMap> ret=create_density_map(obb, in->get_spacing()*scaling);
  for (unsigned int i=0; i< ret->get_number_of_voxels(); ++i) {
    algebra::VectorD<3> v= get_voxel_center(ret, i);
    double d= get_density(in, v);
    ret->set_value(i, d);
  }
  IMP_LOG(TERSE, "Resample from " << in->get_name() << " with spacing "
          << in->get_spacing() << " vs "
          << ret->get_spacing()
          << " and with top " << in->get_top() << " vs " << ret->get_top()
          << " with min/max "
          << *std::min_element(in->get_data(),
                               in->get_data()+in->get_number_of_voxels())
          << "..." << *std::max_element(in->get_data(),
                                        in->get_data()
                                        +in->get_number_of_voxels())
          << " vs " << *std::min_element(ret->get_data(),
                                         ret->get_data()
                                         +ret->get_number_of_voxels())
          << "..." << *std::max_element(ret->get_data(),
                                        ret->get_data()
                                        +ret->get_number_of_voxels())
          << std::endl);
  IMP_LOG(TERSE, "Old map was " << in->get_header()->get_nx() << " "
          << in->get_header()->get_ny() << " " << in->get_header()->get_nz()
          << std::endl);
  if (in->get_header()->get_has_resolution()) {
    ret->get_header_writable()
      ->set_resolution(std::max<double>(in->get_header()->get_resolution(),
                                        ret->get_spacing()));
  }
  ret->set_name(in->get_name() +" resampled");
  return ret.release();
}

IMPEM_END_NAMESPACE
