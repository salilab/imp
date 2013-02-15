/**
 *  \file DensityMap.cpp
 *  \brief Class for handling density maps.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/DensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/XplorReaderWriter.h>
#include <IMP/em/EMReaderWriter.h>
#include <IMP/em/SpiderReaderWriter.h>

IMPEM_BEGIN_NAMESPACE
namespace {
  template <class T>
   bool is_initialize(const T &t) {
     if (std::numeric_limits<T>::has_signaling_NaN) {
       return (base::isnan(t));
     } else if (std::numeric_limits<T>::has_quiet_NaN) {
       return (base::isnan(t));
     } else if (std::numeric_limits<T>::has_infinity) {
       return t > std::numeric_limits<T>::max();
     } else {
       // numerical limits for int and double have completely
       // different meanings of max/min
       return (t <= -std::numeric_limits<T>::max());
     }
  }
}

DensityMap* create_density_map(const DensityMap *other) {
  Pointer<DensityMap> ret = new DensityMap(*(other->get_header()));
  emreal* new_data = ret->get_data();
  emreal* other_data = other->get_data();
  long size=other->get_number_of_voxels();
  std::copy(other_data, other_data+size, new_data);
  return ret.release();
}

DensityMap *create_density_map(const algebra::BoundingBox3D &bb,
                                 double spacing) {
    Pointer<DensityMap> ret(new DensityMap());
    unsigned int n[3];
    algebra::Vector3D wid= bb.get_corner(1)-bb.get_corner(0);
    for (unsigned int i=0; i< 3; ++i) {
      n[i]= static_cast<int>(std::ceil(wid[i]/spacing));
    }
    ret->set_void_map(n[0], n[1], n[2]);
    ret->set_origin(bb.get_corner(0));
    ret->update_voxel_size(spacing);
    ret->get_header_writable()->compute_xyz_top();
    ret->set_name("created density map");
    IMP_LOG_TERSE( "Created map with dimensions " << n[0] << " " << n[1]
            << " " << n[2] << " and spacing " << ret->get_spacing()
            << std::endl);
    return ret.release();
  }


DensityMap *create_density_map(int nx,int ny,int nz,
                                 double spacing) {
    Pointer<DensityMap> ret(new DensityMap());
    ret->set_void_map(nx,ny,nz);
    ret->update_voxel_size(spacing);
    ret->get_header_writable()->compute_xyz_top();
    ret->set_name("created density map");
    IMP_LOG_TERSE( "Created map with dimensions " << nx << " " << ny
            << " " << nz << " and spacing " << ret->get_spacing()
            << std::endl);
    return ret.release();
  }


DensityMap::DensityMap(std::string name): Object(name)
{
  loc_calculated_ = false;
  normalized_ = false;
  rms_calculated_ = false;
}


DensityMap::DensityMap(const DensityHeader &header,
                       std::string name): Object(name){
  header_ = header;
  header_.compute_xyz_top(true);
  //allocate the data
  long nvox = get_number_of_voxels();
  data_.reset(new emreal[nvox]);
  loc_calculated_=false;
  calc_all_voxel2loc();
}

void DensityMap::set_void_map(int nx,int ny,int nz) {
  long nvox = nx*ny*nz;
  data_.reset(new emreal[nvox]);
  for (long i=0;i<nvox;i++) {
    data_[i]=0.0;
  }
  header_.update_map_dimensions(nx,ny,nz);
}

namespace {
  MapReaderWriter *create_reader_writer_from_name(std::string name) {
    IMP_USAGE_CHECK(name.rfind('.') != std::string::npos,
                  "No suffix in file name: " << name);
    std::string suf=name.substr(name.rfind('.'));
    if (suf == ".mrc") {
      return new MRCReaderWriter();
    } else if (suf == ".em") {
      return new EMReaderWriter();
    } else if (suf == ".vol") {
      return new SpiderMapReaderWriter();
    } else if (suf == ".xplor") {
      return new XplorReaderWriter();
    } else {
      IMP_THROW("Unable to determine type for file "<< name
                << " with suffix " << suf,
                IOException);
    }
  }
}

DensityMap* read_map(std::string filename) {
  Pointer<MapReaderWriter> rw= create_reader_writer_from_name(filename);
  return read_map(filename, rw);
}

void write_map(DensityMap *dm, std::string  filename) {
  Pointer<MapReaderWriter> rw= create_reader_writer_from_name(filename);
  write_map(dm, filename, rw);
}

void DensityMap::update_header() {
  if (! is_initialize(header_.get_nx())) {
  }
  if (! is_initialize(header_.get_ny())) {
  }
  if (! is_initialize(header_.get_nz())) {
  }
  IMP_INTERNAL_CHECK(is_initialize(header_.get_data_type()),
      "The data_type of the map header is not initialized");
  if (! is_initialize(header_.nxstart)){
  }
  if (! is_initialize(header_.nystart)){
  }
  if (! is_initialize(header_.nzstart)){
  }
  if (! is_initialize(header_.mx)){
  }
  if (! is_initialize(header_.my)){
  }
  if (! is_initialize(header_.mz)){
  }
  if (! is_initialize(header_.xlen)){
  }
  if (! is_initialize(header_.ylen)){
  }
  if (! is_initialize(header_.zlen)){
  }
  if (! is_initialize(header_.alpha)){
  }
  if (! is_initialize(header_.beta)){
  }
  if (! is_initialize(header_.gamma)){
  }
  if (! is_initialize(header_.mapc)){
  }
  if (! is_initialize(header_.mapr)){
  }
  if (! is_initialize(header_.maps)){
  }
  if (! is_initialize(header_.dmin)){
    header_.dmin = get_min_value();
  }
  if (! is_initialize(header_.dmax)){
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

DensityMap* read_map(std::string filename, MapReaderWriter *reader)
{
  // TODO: we need to decide who does the allocation ( mapreaderwriter or
  // density)? if we keep the current implementation ( mapreaderwriter )
  // we need to pass a pointer to data_
  Pointer<MapReaderWriter> ptr(reader);
  Pointer<DensityMap> m= new DensityMap();
  float *f_data=nullptr;
  reader->read(filename.c_str(), &f_data, m->header_);
  reader->set_was_used(true);
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
  IMP_LOG_TERSE( "Read range is "
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

void write_map(DensityMap *d, std::string  filename, MapReaderWriter *writer)
{
  IMP::Pointer<MapReaderWriter> pt(writer);
  writer->set_was_used(true);
  d->set_was_used(true);
  boost::scoped_array<float> f_data;
  d->real2float(d->data_.get(), f_data);
  writer->write(filename.c_str(), f_data.get(), d->header_);
}

long DensityMap::get_number_of_voxels() const {
  return header_.get_number_of_voxels();
}

float DensityMap::get_location_in_dim_by_voxel(long index, int dim) const
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

long DensityMap::get_voxel_by_location(float x, float y, float z) const {
  IMP_USAGE_CHECK(is_part_of_volume(x,y,z),
            "The point is not part of the grid");
  int ivoxx=get_dim_index_by_location(x,0);
  int ivoxy=get_dim_index_by_location(y,1);
  int ivoxz=get_dim_index_by_location(z,2);
  return xyz_ind2voxel(ivoxx,ivoxy,ivoxz);
}

bool DensityMap::is_xyz_ind_part_of_volume(int ix,int iy,int iz) const
{
  return ( ix>=0 && ix<header_.get_nx() &&
      iy>=0 && iy<header_.get_ny() &&
           iz>=0 && iz<header_.get_nz() );
}


bool DensityMap::is_part_of_volume(float x,float y,float z) const
{
  int x_ind,y_ind,z_ind;
  x_ind=get_dim_index_by_location(x,0);
  y_ind=get_dim_index_by_location(y,1);
  z_ind=get_dim_index_by_location(z,2);
  return (x_ind>=0) && (x_ind<header_.get_nx()) &&
    (y_ind>=0) && (y_ind<header_.get_ny()) &&
    (z_ind>=0) && (z_ind<header_.get_nz());
}

emreal DensityMap::get_value(float x, float y, float z) const {
  return data_[get_voxel_by_location(x,y,z)];
}

emreal DensityMap::get_value(long index) const {
  IMP_USAGE_CHECK(index >= 0 && index < get_number_of_voxels(),
            "The index " << index << " is not part of the grid"
            <<"[0,"<<get_number_of_voxels()<<"]\n");
  return data_[index];
}

void DensityMap::set_value(long index,emreal value) {
  IMP_USAGE_CHECK(index >= 0 && index < get_number_of_voxels(),
            "The index " << index << " is not part of the grid"
            <<"[0,"<<get_number_of_voxels()<<"]\n");
  data_[index]=value;
  normalized_ = false;
  rms_calculated_ = false;
}

void DensityMap::set_value(float x, float y, float z,emreal value) {
  data_[get_voxel_by_location(x,y,z)]=value;
  normalized_ = false;
  rms_calculated_ = false;
}


void DensityMap::reset_all_voxel2loc() {
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

  emreal max_value=- std::numeric_limits<emreal>::max();
  emreal min_value=-max_value;
  float inv_std = 1.0/calcRMS();
  float mean = header_.dmean;
  long nvox = get_number_of_voxels();

  for (long ii=0;ii<nvox;ii++) {
    data_[ii] = (data_[ii] - mean) * inv_std;
    max_value=std::max(max_value, data_[ii]);
    min_value=std::min(min_value, data_[ii]);
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
  long  nvox = get_number_of_voxels();
  emreal meanval = .0;
  emreal stdval = .0;

  for (long ii=0;ii<nvox;ii++) {
    meanval += data_[ii];
    stdval += data_[ii] * data_[ii];
  }
  IMP_LOG_VERBOSE(
          "in calc RMS meanval:"<<meanval<<" stdval:"<<stdval<<std::endl);
  header_.dmin=get_min_value();
  header_.dmax=get_max_value();
  IMP_LOG_VERBOSE("in calc RMS min:"<< header_.dmin
          <<" max:"<< header_.dmax<<std::endl);
  meanval /=  nvox;
  header_.dmean = meanval;

  stdval = sqrt(stdval/nvox-meanval*meanval);
  header_.rms = stdval;
  rms_calculated_=true;
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
  reset_all_voxel2loc();
  calc_all_voxel2loc();
}




bool DensityMap::same_origin(const DensityMap *other) const
{
  if( fabs(get_header()->get_xorigin()
           -other->get_header()->get_xorigin())<EPS &&
      fabs(get_header()->get_yorigin()
           -other->get_header()->get_yorigin())<EPS &&
      fabs(get_header()->get_zorigin()
           -other->get_header()->get_zorigin())<EPS)
    return true;
  return false;
}

bool DensityMap::same_dimensions(const DensityMap *other) const
{
  return ((get_header()->get_nx()==other->get_header()->get_nx()) &&
          (get_header()->get_ny()==other->get_header()->get_ny()) &&
          (get_header()->get_nz()==other->get_header()->get_nz()));
}

bool DensityMap::same_voxel_size(const DensityMap *other) const
{
  if(fabs(get_header()->get_spacing()
          - other->get_header()->get_spacing()) < EPS)
    return true;
  return false;
}

algebra::Vector3D DensityMap::get_centroid(emreal threshold)  const{
  IMP_LOG_VERBOSE(
          "Max value:"<<get_max_value()<<" thre:"<<threshold<<std::endl);
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
    x_centroid += get_location_in_dim_by_voxel(i,0);
    y_centroid += get_location_in_dim_by_voxel(i,1);
    z_centroid += get_location_in_dim_by_voxel(i,2);
    counter += 1;
  }
  //counter will not be 0 since we checked that threshold is within
  //the map densities.
  x_centroid /= counter;
  y_centroid /= counter;
  z_centroid /= counter;
  return algebra::Vector3D(x_centroid,y_centroid,z_centroid);
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
      x=get_location_in_dim_by_voxel(i,0);
      y=get_location_in_dim_by_voxel(i,1);
      z=get_location_in_dim_by_voxel(i,2);
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
double get_sum(const DensityMap *m1) {
  long size = m1->get_header()->get_number_of_voxels();
  emreal *data = m1->get_data();
  double sum_all=0.;
  for (long i=0;i<size;i++){
    sum_all+=data[i];
  }
  return sum_all;
}
void DensityMap::add(const DensityMap *other) {
  //check that the two maps have the same dimensions
  IMP_USAGE_CHECK(same_voxel_size(other),
            "The voxel sizes of the two maps differ ( "
            << header_.get_spacing() << " != "
            << other->header_.get_spacing());
  /*  IMP_INTERNAL_CHECK(
          get_bounding_box(this).get_contains(get_bounding_box(other)),
          "Other map should be contained in this map\n");*/
  //find the intersecting bounding box
  algebra::BoundingBox3D bb =
    get_intersection(em::get_bounding_box(this),em::get_bounding_box(other));
  IMP_IF_LOG(VERBOSE){
    IMP_LOG_VERBOSE("bounding boxes:"<<std::endl);
    IMP_LOG_WRITE(VERBOSE,em::get_bounding_box(other).show());
    IMP_LOG_WRITE(VERBOSE,em::get_bounding_box(this).show());
    IMP_LOG_WRITE(VERBOSE,bb.show());
  }
  int x_orig_ind=0;int y_orig_ind=0;int z_orig_ind=0;
  int x_top_ind=header_.get_nx();
  int y_top_ind=header_.get_ny();
  int z_top_ind=header_.get_nz();
  x_orig_ind=get_dim_index_by_location(bb.get_corner(0)[0],0);
  y_orig_ind=get_dim_index_by_location(bb.get_corner(0)[1],1);
  z_orig_ind=get_dim_index_by_location(bb.get_corner(0)[2],2);
  x_top_ind=get_dim_index_by_location(bb.get_corner(1)[0],0);
  y_top_ind=get_dim_index_by_location(bb.get_corner(1)[1],1);
  z_top_ind=get_dim_index_by_location(bb.get_corner(1)[2],2);


  int ox_orig_ind,oy_orig_ind,oz_orig_ind;
  ox_orig_ind=other->get_dim_index_by_location(bb.get_corner(0)[0],0);
  oy_orig_ind=other->get_dim_index_by_location(bb.get_corner(0)[1],1);
  oz_orig_ind=other->get_dim_index_by_location(bb.get_corner(0)[2],2);

  long my_znxny,other_znxny,my_znxny_ynx,other_znxny_ynx;
  long my_nxny=header_.get_nx()*header_.get_ny();
  long other_nxny=other->header_.get_nx()*other->header_.get_ny();
  for(int iz=0;iz<(z_top_ind-z_orig_ind);iz++){
    //we do not add +1 to avoid numerical problems
    my_znxny=(iz+z_orig_ind) * my_nxny;
    other_znxny=(iz+oz_orig_ind) * other_nxny;
    for(int iy=0;iy<(y_top_ind-y_orig_ind);iy++){
      my_znxny_ynx = my_znxny + (iy+y_orig_ind) * header_.get_nx();
      other_znxny_ynx = other_znxny + (iy+oy_orig_ind)
         * other->header_.get_nx();
      for(int ix=0;ix<(x_top_ind-x_orig_ind);ix++){
        data_[my_znxny_ynx+ix+x_orig_ind]+=
          other->data_[other_znxny_ynx+ix+ox_orig_ind];
      }
    }
  }
}

void DensityMap::pick_max(const DensityMap *other) {
  //check that the two maps have the same dimensions
  IMP_USAGE_CHECK(same_voxel_size(other),
            "The voxel sizes of the two maps differ ( "
            << header_.get_spacing() << " != "
            << other->header_.get_spacing());
  //TODO - add dimension test
  long size = header_.get_number_of_voxels();
  emreal *other_data = other->get_data();
  for (long i=0;i<size;i++){
    if (data_[i]<other_data[i]) {
      data_[i]=other_data[i];
    }
  }
}


void DensityMap::pad(int nx, int ny, int nz,float val) {

  IMP_USAGE_CHECK(nx >= header_.get_nx() &&
                  ny >= header_.get_ny() &&
                  nz >= header_.get_nz(),
            "The requested volume is smaller than the existing one");

  long new_size = nx*ny*nz;
  long cur_size = get_number_of_voxels();
  reset_all_voxel2loc();
  calc_all_voxel2loc();
  boost::scoped_array<emreal> new_data(new emreal[new_size]);
  for (long i = 0; i < new_size; i++) {
    new_data[i] = val;
  }
  int new_vox_x,new_vox_y,new_vox_z;
  long new_vox;
  for (long i = 0; i <  cur_size; i++) {
    float x = get_location_in_dim_by_voxel(i,0);
    float y = get_location_in_dim_by_voxel(i,1);
    float z = get_location_in_dim_by_voxel(i,2);
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
  reset_all_voxel2loc();
  calc_all_voxel2loc();
}


Float approximate_molecular_mass(DensityMap* d, Float threshold) {
  long counter=0;//number of voxles above the threshold
  for(long l=0;l<d->get_number_of_voxels();l++) {
    if (d->get_value(l) > threshold) {
      ++counter;
     }
   }
  double cube_factor=1.21*1.21*1.21;
  double cube_spacing=d->get_spacing()*d->get_spacing()*d->get_spacing();
  return cube_spacing*counter/cube_factor;
 }



/* Daniel's helpers */
namespace {
  inline algebra::Vector3D get_voxel_center(const DensityMap *map,
                                     unsigned int v) {
    return algebra::Vector3D(map->get_location_by_voxel(v));
  }

  DensityMap *create_density_map(const algebra::Vector3D &origin,
                                 int nx,int ny, int nz,
                                 double spacing) {
    Pointer<DensityMap> ret(new DensityMap());
    ret->set_void_map(nx,ny,nz);
    ret->set_origin(origin);
    ret->update_voxel_size(spacing);
    ret->get_header_writable()->compute_xyz_top();
    IMP_LOG_TERSE( "Created map with dimensions " << nx << " " << ny
            << " " << nz << " and spacing " << spacing
            << std::endl);
    return ret.release();
  }


  /* Surround the density map with an extra set of samples assumed to
     be 0.
  */
  inline double get_value(const DensityMap *m, int xi,
                          int yi, int zi) {
    if (xi < 0 || yi < 0 || zi < 0) return 0.0;
    else if (xi >= m->get_header()->get_nx()
             || yi >= m->get_header()->get_ny()
             || zi >= m->get_header()->get_nz()) return 0.0;
    else {
      unsigned int loc= m->xyz_ind2voxel(xi, yi, zi);
      double v= m->get_value(loc);
      return v;
    }
  }

  inline void compute_voxel(const DensityMap *m, const algebra::Vector3D &v,
                            int *ivox, algebra::Vector3D &remainder) {
    const double iside= 1.0/m->get_spacing();
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
  // unused
  /*
  inline unsigned int get_n(const DensityMap *m, unsigned int dim) {
    switch (dim) {
    case 0:
      return m->get_header()->get_nx();
    case 1:
      return m->get_header()->get_ny();
    default:
      return m->get_header()->get_nz();
    }
    }*/
}


algebra::BoundingBox3D get_bounding_box(const DensityMap* d,Float threshold) {
  algebra::BoundingBox3D ret;
  for(long l=0;l<d->get_number_of_voxels();l++) {
    if (d->get_value(l) > threshold) {
      algebra::Vector3D v(get_voxel_center(d, l));
      ret+= v;
     }
  }
  // make sure it includes the whole voxels
  return ret+=d->get_spacing()/2.0;
}


double get_density(const DensityMap *m, const algebra::Vector3D &v) {
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
  algebra::Vector3D r;
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

namespace {
void get_transformed_internal(const DensityMap *in,
          const algebra::Transformation3D &tr,
          DensityMap *ret){
  algebra::Transformation3D tri= tr.get_inverse();
  algebra::Rotation3D tri_rot = tri.get_rotation();
  algebra::Vector3D rotmat0 = tri_rot.get_rotation_matrix_row(0);
  algebra::Vector3D rotmat1 = tri_rot.get_rotation_matrix_row(1);
  algebra::Vector3D rotmat2 = tri_rot.get_rotation_matrix_row(2);
  algebra::Vector3D tri_trans = tri.get_translation();
  unsigned int size=ret->get_number_of_voxels();
  for (unsigned int i=0; i< size; ++i) {
    algebra::Vector3D tpt=get_voxel_center(ret, i);
    // equivalent to pt= tri.get_transformed(tpt), but always uses
    // the rotation matrix for a faster inner loop
    algebra::Vector3D pt(tpt * rotmat0 + tri_trans[0],
                         tpt * rotmat1 + tri_trans[1],
                         tpt * rotmat2 + tri_trans[2]);
    double d = get_density(in, pt);
    ret->set_value(i, d);
  }
  ret->set_name(std::string("transformed ")+ in->get_name());
  if (in->get_header()->get_has_resolution()) {
    ret->get_header_writable()
      ->set_resolution(in->get_header()->get_resolution());
  }
  ret->calcRMS();
}
}

DensityMap *get_transformed(const DensityMap *in,
                            const algebra::Transformation3D &tr,
                            double threshold) {
  algebra::BoundingBox3D nbb= get_bounding_box(in, threshold);
  IMP::Pointer<DensityMap> ret(create_density_map(nbb,
                                 in->get_header()->get_spacing()));
  get_transformed_internal(in,tr,ret);
  return ret.release();
}

DensityMap *get_transformed(DensityMap *in,
                            const algebra::Transformation3D &tr){
  IMP::Pointer<DensityMap> ret(create_density_map(
        in->get_origin(),
        in->get_header()->get_nx(),
        in->get_header()->get_ny(),
        in->get_header()->get_nz(),
        in->get_header()->get_spacing()));
  get_transformed_internal(in,tr,ret);
  return ret.release();
}

DensityMap* get_resampled(DensityMap *in, double scaling) {
  algebra::BoundingBox3D obb= get_bounding_box(in,
                                         -std::numeric_limits<float>::max());
  Pointer<DensityMap> ret=create_density_map(obb, in->get_spacing()*scaling);
  for (int i=0; i< ret->get_number_of_voxels(); ++i) {
    algebra::Vector3D v= get_voxel_center(ret, i);
    double d= get_density(in, v);
    ret->set_value(i, d);
  }
  IMP_LOG_TERSE( "Resample from " << in->get_name() << " with spacing "
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
  IMP_LOG_TERSE( "Old map was "
          << in->get_header()->get_nx() << " "
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
void DensityMap::copy_map(const DensityMap *other) {
  header_ = other->header_;
  long size = get_number_of_voxels();
  data_.reset(new emreal[size]);
  std::copy(other->data_.get(), other->data_.get()+size, data_.get());
  loc_calculated_ = other->loc_calculated_;
  if (loc_calculated_) {
    x_loc_.reset(new float[size]);
    y_loc_.reset(new float[size]);
    z_loc_.reset(new float[size]);
    std::copy(other->x_loc_.get(), other->x_loc_.get()+size, x_loc_.get());
    std::copy(other->y_loc_.get(), other->y_loc_.get()+size, y_loc_.get());
    std::copy(other->z_loc_.get(), other->z_loc_.get()+size, z_loc_.get());
  } else {
    x_loc_.reset();
    y_loc_.reset();
    z_loc_.reset();
  }
  data_allocated_ = other->data_allocated_;
  normalized_ = other->normalized_;
  rms_calculated_ = other->rms_calculated_;
}

void get_transformed_into(const DensityMap *from,
   const algebra::Transformation3D &tr,
   DensityMap *into,
   bool calc_rms) {
  algebra::BoundingBox3D obb(from->get_origin(),from->get_top());
  OwnerPointer<DensityMap> nmap(create_density_map(obb,into->get_spacing()));
  into->copy_map(nmap);
  get_transformed_internal(from,tr,into);
  into->get_header_writable()->compute_xyz_top();
  if (calc_rms) {
    into->calcRMS();
  }
}

void get_transformed_into2(const DensityMap *from,
   const algebra::Transformation3D &tr,
                           DensityMap *into){
  algebra::BoundingBox3D obb(from->get_origin(),from->get_top());
  get_transformed_internal(from,tr,into);
  into->get_header_writable()->compute_xyz_top();
}

DensityMap* DensityMap::pad_margin(int mrg_x, int mrg_y,
                                   int mrg_z,float /*val*/) {
  Pointer<DensityMap> ret(new DensityMap(header_));
  //calculate the new extent
  int new_ext[3];
  new_ext[0]=header_.get_nx()+mrg_x*2;
  new_ext[1]=header_.get_ny()+mrg_y*2;
  new_ext[2]=header_.get_nz()+mrg_z*2;
  ret->set_void_map(new_ext[0],new_ext[1],new_ext[2]);
  ret->set_origin(
    get_origin()-header_.get_spacing()*algebra::Vector3D(mrg_x,mrg_y,mrg_z));
  const DensityHeader *new_header = ret->get_header();
  long z_term_curr,z_term_new,zy_term_curr,zy_term_new,curr_ind,new_ind;
  int nxny=header_.get_nx()*header_.get_ny();
  int n_nxny=new_header->get_ny()*new_header->get_nx();
  int nx=header_.get_nx();
  int n_nx=new_header->get_nx();
  emreal *new_data = ret->get_data();
  long n_xyz=get_number_of_voxels();
  long new_n_xyz=ret->get_number_of_voxels();
  IMP_UNUSED(n_xyz);
  IMP_UNUSED(new_n_xyz);
  for(int iz=0;iz<header_.get_nz();iz++){ //z slowest
    z_term_curr = iz*nxny;
    z_term_new = (iz+mrg_z)*n_nxny;
    for(int iy=0;iy<header_.get_ny();iy++){
      zy_term_curr = iy*nx + z_term_curr;
      zy_term_new = (iy+mrg_y)*n_nx + z_term_new;
      for(int ix=0;ix<nx;ix++){
        curr_ind = zy_term_curr+ix;
        new_ind = zy_term_new+ix+mrg_x;
        IMP_INTERNAL_CHECK(new_ind<new_n_xyz,
                           "Index problem with new map!"<<std::endl);
        IMP_INTERNAL_CHECK(curr_ind<n_xyz,
                           "Index problem with old map!"<<std::endl);
        new_data[new_ind]=data_[curr_ind];
      }
    }
  }
  return ret.release();
}
// statistics::Histogram
// get_density_histogram(const DensityMap *dmap, float threshold,
//                       int num_bins) {
//   statistics::Histogram hist(threshold-EPS,dmap->get_header()->dmax+.1,
//                              num_bins);
//   for(long v_ind=0;v_ind<dmap->get_number_of_voxels();v_ind++) {
//     if (dmap->get_value(v_ind) > threshold) {
//       hist.add(dmap->get_value(v_ind));
//     }
//   }
//   return hist;
// }
int DensityMap::lower_voxel_shift(
  emreal loc, emreal kdist, emreal orig, int ndim) const {
  int imin;
  imin = static_cast<int>(std::floor((loc-kdist-orig) / header_.get_spacing()));
  //imin = static_cast<int>(std::floor((loc-kdist) / header_.get_spacing()));
  //bookkeeping
  if (imin < 0)
    imin = 0;
  if (imin > ndim-1)
    imin = ndim-1;
  return imin;
}

int DensityMap::upper_voxel_shift(emreal loc, emreal kdist,
                                  emreal orig, int ndim) const {
  int imax;
  imax = static_cast<int>(std::floor((loc+kdist-orig)/ header_.get_spacing()));
  //  imax = static_cast<int>(std::floor((loc+kdist)/ header_.get_spacing()));
  //bookkeeping
  if (imax < 0) imax = 0;
  if (imax > ndim-1) imax = ndim-1;
  return imax;
}

float DensityMap::get_maximum_value_in_xy_plane(int z_ind) {
  IMP_USAGE_CHECK(z_ind<header_.get_nz(),
                  "Z index is out of range\n");
  float max_val=-INT_MAX;
  int temp_ind;
  int z_val=z_ind*header_.get_ny()*header_.get_nx();
  for(int iy=0;iy<header_.get_ny();iy++){
    temp_ind = z_val+iy*header_.get_nx();
    for(int ix=0;ix<header_.get_nx();ix++){
      if (get_value(temp_ind+ix)>max_val) {
        max_val = get_value(temp_ind+ix);
      }
    }
  }
  return max_val;
}

float DensityMap::get_maximum_value_in_xz_plane(int y_ind) {
  IMP_USAGE_CHECK(y_ind<header_.get_ny(),
                  "Y index is out of range\n");
  float max_val=-INT_MAX;
  int temp_ind;
  int y_val=y_ind*header_.get_nx();
  for(int iz=0;iz<header_.get_nz();iz++){
    temp_ind = y_val+iz*header_.get_nx()*header_.get_ny();
    for(int ix=0;ix<header_.get_nx();ix++){
      if (get_value(temp_ind+ix)>max_val) {
        max_val = get_value(temp_ind+ix);
      }
    }
  }
  return max_val;
}


float DensityMap::get_maximum_value_in_yz_plane(int x_ind) {
  IMP_USAGE_CHECK(x_ind<header_.get_ny(),
                  "X index is out of range\n");
  float max_val=-INT_MAX;
  int temp_ind;
  for(int iz=0;iz<header_.get_nz();iz++){
    temp_ind = iz*header_.get_nx()*header_.get_ny();
    for(int iy=0;iy<header_.get_ny();iy++){
      if (get_value(temp_ind+iy*header_.get_nx()+x_ind)>max_val) {
        max_val = get_value(temp_ind+iy*header_.get_nx()+x_ind);
      }
    }
  }
  return max_val;
}

DensityMap* DensityMap::get_cropped(const algebra::BoundingBox3D &bb) {
  //snap the input bb to the density dimensions
  algebra::Vector3D ll,ur;
  ll=bb.get_corner(0);ur=bb.get_corner(1);
  if (!is_part_of_volume(ll)){
    ll=get_origin();
  }
  if (!is_part_of_volume(ur)){
    ur=get_top();
  }
  algebra::BoundingBox3D snapped_bb(ll,ur);
  IMP::Pointer<DensityMap> cropped_dmap =
    create_density_map(snapped_bb,get_spacing());
  //copy resolution
  cropped_dmap->get_header_writable()->set_resolution(header_.get_resolution());
  //now fill the density
  const DensityHeader *c_header = cropped_dmap->get_header();
  long z_temp,zy_temp,c_z_temp,c_zy_temp;
  int c_nx,c_ny;
  //the bounding box in the original map
  int z_start,y_start,x_start;
  int z_end,y_end,x_end;
  x_start=get_dim_index_by_location(snapped_bb.get_corner(0),0);
  y_start=get_dim_index_by_location(snapped_bb.get_corner(0),1);
  z_start=get_dim_index_by_location(snapped_bb.get_corner(0),2);
  x_end=get_dim_index_by_location(snapped_bb.get_corner(1),0);
  y_end=get_dim_index_by_location(snapped_bb.get_corner(1),1);
  z_end=get_dim_index_by_location(snapped_bb.get_corner(1),2);
  c_nx=c_header->get_nx();c_ny=c_header->get_ny();
  for(int iz=z_start;iz<z_end;iz++){ //z slowest
    z_temp = iz*header_.get_nx()*header_.get_ny();
    c_z_temp = (iz-z_start)*c_nx*c_ny;
    for(int iy=y_start;iy<y_end;iy++){
      zy_temp = z_temp+iy*header_.get_nx();
      c_zy_temp = c_z_temp+(iy-y_start)*c_nx;
      for(int ix=x_start;ix<x_end;ix++){
        cropped_dmap->set_value(c_zy_temp+(ix-x_start),
                            get_value(zy_temp+ix));
      }
    }
  }
  cropped_dmap->calcRMS();
  return cropped_dmap.release();
}

DensityMap* DensityMap::get_cropped(float threshold){
  IMP_USAGE_CHECK(threshold>get_min_value()-EPS,
                  "The input threshold is too small\n");
  algebra::BoundingBox3D bb = get_bounding_box(this,threshold);
  return get_cropped(bb);
}

int DensityMap::get_dim_index_by_location(float loc_val,
                                          int ind) const {
  return static_cast<int>(std::floor((loc_val-get_origin()[ind])
                                     /header_.get_spacing()));
}
int DensityMap::get_dim_index_by_location(const algebra::Vector3D &v,
                                          int ind) const {
  IMP_INTERNAL_CHECK((ind>-1) && (ind<3),"index out of range\n");
  IMP_INTERNAL_CHECK(is_part_of_volume(v),
                     "location outside of map boundaries\n");
  return get_dim_index_by_location(v[ind],ind);
}


IMPEMEXPORT DensityMap* get_segment(DensityMap *from_map,
                                    int nx_start,int nx_end,
                                    int ny_start,int ny_end,
                                    int nz_start,int nz_end) {
  const DensityHeader *from_header = from_map->get_header();
  int from_nx=from_header->get_nx();
  int from_ny=from_header->get_ny();
  IMP_CHECK_CODE(int from_nz=from_header->get_nz());
  int to_nx=nx_end-nx_start+1;
  int to_ny=ny_end-ny_start+1;
  int to_nz=nz_end-nz_start+1;
  IMP_IF_CHECK(USAGE) {

  IMP_USAGE_CHECK(nx_start>=0 && nx_start<from_nx,
                  "nx start index is out of boundaries\n");
  IMP_USAGE_CHECK(nx_end>=0   && nx_end<from_nx,
                  "nx end index is out of boundaries\n");
  IMP_USAGE_CHECK(ny_start>=0 && ny_start<from_ny,
                  "ny start index is out of boundaries\n");
  IMP_USAGE_CHECK(ny_end>=0   && ny_end<from_ny,
                  "ny end index is out of boundaries\n");
  IMP_USAGE_CHECK(nz_start>=0 && nz_start<from_nz,
                  "nz start index is out of boundaries\n");
  IMP_USAGE_CHECK(nz_end>=0   && nz_end<from_nz,
                  "nz end index is out of boundaries\n");
  }
  //create a new map
  Pointer<DensityMap> to_map(create_density_map(to_nx,to_ny,to_nz,
                                                from_header->get_spacing()));
  to_map->set_origin(
   from_map->get_location_by_voxel(
                from_map->xyz_ind2voxel(nx_start,ny_start,nz_start)));
  emreal *to_data=to_map->get_data();
  emreal *from_data=from_map->get_data();
  //copy data
  long from_ind_z,to_ind_z;
  long from_ind,to_ind;
  for(int iz=nz_start;iz<=nz_end;iz++){
    from_ind_z=iz*from_nx*from_ny;
    to_ind_z=(iz-nz_start)*to_nx*to_ny;
    for(int iy=ny_start;iy<=ny_end;iy++){
      from_ind=from_ind_z+iy*from_nx+nx_start;
      to_ind=to_ind_z+(iy-ny_start)*to_nx;
      for(int ix=nx_start;ix<=nx_end;ix++){
        to_data[to_ind]=from_data[from_ind];
        ++to_ind;++from_ind;
      }
    }
  }
  return to_map.release();
}

DensityMap* binarize(DensityMap *orig_map,
                     float threshold,bool reverse) {
  const DensityHeader *header = orig_map->get_header();
  //create a new map
  Pointer<DensityMap> bin_map = create_density_map(orig_map);
  bin_map->reset_data(0.);
  emreal *orig_data=orig_map->get_data();
  emreal *bin_data=bin_map->get_data();
  for(long i=0;i<header->get_number_of_voxels();i++){
    if (!reverse) {
      if (orig_data[i]<threshold) {
        bin_data[i]=0.;
      } else {
        bin_data[i]=1.;
      }
    }
    else {
      if (orig_data[i]>threshold) {
        bin_data[i]=0.;
      } else {
        bin_data[i]=1.;
      }
    }
  }
  return bin_map.release();
}

DensityMap* get_threshold_map(DensityMap *orig_map,
                     float threshold) {
  const DensityHeader *header = orig_map->get_header();
  //create a new map
  Pointer<DensityMap> ret(create_density_map(header->get_nx(),
                                             header->get_ny(),header->get_nz(),
                                             header->get_spacing()));
  ret->set_origin(orig_map->get_origin());
  emreal *orig_data=orig_map->get_data();
  emreal *ret_data=ret->get_data();
  for(long i=0;i<header->get_number_of_voxels();i++){
    if (orig_data[i]<threshold) {
      ret_data[i]=0.;
    } else {
      ret_data[i]=orig_data[i];
    }
  }
  return ret.release();
}



double convolute(const DensityMap *m1,const DensityMap *m2){
  const DensityHeader *h1=m1->get_header();
  const DensityHeader *h2=m2->get_header();
  emreal *d1=m1->get_data();
  emreal *d2=m2->get_data();
  float voxel_size = m1->get_spacing();
  int ivoxx_shift = (int)floor((h2->get_xorigin()
                                - h1->get_xorigin())
                               / voxel_size);
  int ivoxy_shift = (int)floor((h2->get_yorigin()
                                - h1->get_yorigin())
                               / voxel_size);
  int ivoxz_shift = (int)floor((h2->get_zorigin()
                                - h1->get_zorigin())
                               / voxel_size);
  long j; // m1 index
  long i; // m2 index
  long nvox=m1->get_number_of_voxels();
  // calculate the shift in index of the origin of m2 in m1
  // ( j can be negative)
  j = ivoxz_shift * h1->get_nx() * h1->get_ny()
    + ivoxy_shift * h1->get_nx() + ivoxx_shift;
  double conv=0.;
  for (i=0;i<nvox;i++) {
    if (j + i >= 0 && j + i < nvox)  {
      conv += d1[j+i] * d2[i];
    }
  }
  return conv;
}

DensityMap* multiply(const DensityMap *m1,
                     const DensityMap *m2){
  const DensityHeader *header=m1->get_header();
  Pointer<DensityMap> m_map(
    create_density_map(header->get_nx(),
                       header->get_ny(),header->get_nz(),
                       header->get_spacing()));
  m_map->set_origin(m2->get_origin());
  emreal *data1=m1->get_data();
  emreal *data2=m2->get_data();
  emreal *new_data=m_map->get_data();
  for(long i=0;i<header->get_number_of_voxels();i++){
    new_data[i]=data1[i]*data2[i];
  }
  return m_map.release();
}


DensityMap* get_max_map(DensityMaps maps){
  IMP_USAGE_CHECK(maps.size()>0,"get_max_map should get as input "<<
                  "at least one map\n");
  Pointer<DensityMap> max_map(new DensityMap(*(maps[0]->get_header())));
  max_map->reset_data(-INT_MAX);
  const em::DensityHeader *max_map_h=max_map->get_header();
  for(DensityMaps::const_iterator it = maps.begin();
      it !=maps.end();it++){
      IMP_USAGE_CHECK(max_map->same_dimensions(*it),
                      "all maps should have the same extent\n");
      IMP_USAGE_CHECK(max_map->same_origin(*it),
                      "all maps should have the same origin\n");
  }
  int nx,ny,nz;
  double *max_map_data=max_map->get_data();
  nx=max_map_h->get_nx();
  ny=max_map_h->get_ny();
  nz=max_map_h->get_nz();
  for(long ind=0;ind<nx*ny*nz;ind++){
    for(DensityMaps::const_iterator it = maps.begin();
        it !=maps.end();it++){
      if (max_map_data[ind]<(*it)->get_data()[ind]) {
        max_map_data[ind]=(*it)->get_data()[ind];
      }}}
  return max_map.release();
}
DensityMap* get_segment_by_masking(DensityMap *map_to_segment,
                        DensityMap *mask,
                        float mask_threshold) {
  Pointer<DensityMap> bin_map(binarize(mask,mask_threshold));
  //clean isotlated zeros - to that with conn_comp
  Pointer<DensityMap> ret(multiply(map_to_segment,bin_map));
  std::cout<<"ret:"<<ret->get_min_value()<<","<<ret->get_max_value()<<std::endl;
  return ret.release();
}
//! Get a segment of the map covered by the input points
DensityMap* get_segment(DensityMap *map_to_segment,
                        algebra::Vector3Ds vecs,float radius) {
  IMP_NEW(DensityMap,ret,(*(map_to_segment->get_header())));
  const DensityHeader *header=map_to_segment->get_header();
  emreal *dmap_data=map_to_segment->get_data();
  emreal *segment_data=ret->get_data();
  ret->reset_data(0.);
  int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  int nxny=header->get_nx()*header->get_ny(); int znxny;
  for(int i=0;i<(int)vecs.size();i++) {
    int count=0;
    algebra::Vector3D point=vecs[i];
    calc_local_bounding_box(map_to_segment,
                            vecs[i][0],vecs[i][1],vecs[i][2],
                            radius,
                            iminx, iminy, iminz, imaxx, imaxy, imaxz);
    for (ivoxz=iminz;ivoxz<=imaxz;ivoxz++) {
      znxny=ivoxz * nxny;
      for (ivoxy=iminy;ivoxy<=imaxy;ivoxy++)  {
        // we increment ivox this way to avoid unneceessary multiplication
        // operations.
        ivox = znxny + ivoxy * header->get_nx() + iminx;
        for (ivoxx=iminx;ivoxx<=imaxx;ivoxx++) {
          segment_data[ivox]=dmap_data[ivox];
          ivox++;++count;
        }
      }if (count<3)std::cout<<"Particle "<<i<<" has no data"<<std::endl;}
  }
  return ret.release();
}
void DensityMap::convolute_kernel(DensityMap *other,
                                  double *kernel, int dim_len){
  reset_data(0.);
  emreal *other_data=other->get_data();
  //todo - add a test that lenght is even
  IMP_USAGE_CHECK((dim_len*dim_len*dim_len)>=1,"The input lenght is wrong\n");
  unsigned int margin=(dim_len-1)/2;
  //smooth the density using the kernel
  float val;
  int kernel_ind,ny,nx;
  long map_ind;
  nx=header_.get_nx();
  ny=header_.get_ny();
  for (unsigned int iz=margin;iz<header_.get_nz()-margin;iz++) {
    for (unsigned int iy=margin;iy<header_.get_ny()-margin;iy++) {
      for (unsigned int ix=margin;ix<header_.get_nx()-margin;ix++) {
        map_ind = iz*ny*nx+iy*nx+ix;
        val = other_data[map_ind];
        if (val>EPS) { //smooth this value
          for (int iz2=-margin;iz2<=static_cast<int>(margin);iz2++) {
            for (int iy2=-margin;iy2<=static_cast<int>(margin);iy2++){
              kernel_ind=(iz2+margin)*dim_len*dim_len+(iy2+margin)*dim_len;
              for (int ix2=-margin;ix2<=static_cast<int>(margin);ix2++) {
                data_[(iz+iz2)*ny*nx+
                      (iy+iy2)*nx+ix+ix2]+=
                  val*kernel[kernel_ind+ix2+margin];
              }}} // for iz2,iy2,ix2
        }//if val>EPS
      }}} // for iz,iy,ix
  }

DensityMap* interpolate_map (DensityMap *in_map,double new_spacing) {
  const DensityHeader *in_header = in_map->get_header();
  double in_spacing = in_header->get_spacing();
  double *in_data=in_map->get_data();
  algebra::Vector3D in_orig=in_map->get_origin();
  int in_nx=in_header->get_nx();
  int in_ny=in_header->get_ny();
  int in_nz=in_header->get_nz();

  //round origin voxel index for ret
  int round_ret_orig_x = ceil(in_orig[0]/new_spacing);
  int round_ret_orig_y = ceil(in_orig[1]/new_spacing);
  int round_ret_orig_z = ceil(in_orig[2]/new_spacing);

  //rount top voxel index for ret
  int round_ret_top_x = floor((in_orig[0]+in_spacing*(in_nx-1))/new_spacing);
  int round_ret_top_y = floor((in_orig[1]+in_spacing*(in_ny-1))/new_spacing);
  int round_ret_top_z = floor((in_orig[2]+in_spacing*(in_nz-1))/new_spacing);
  // update grid size
  int ret_nx = round_ret_top_x-round_ret_orig_x+1;
  int ret_ny = round_ret_top_y-round_ret_orig_y+1;
  int ret_nz = round_ret_top_z-round_ret_orig_z+1;
  IMP_INTERNAL_CHECK((ret_nx>2 && ret_ny>2 && ret_nz>2),
                     "Grid too small to interpolate");

  Pointer<DensityMap> ret=create_density_map(ret_nx,ret_ny,ret_nz,new_spacing);
  ret->set_origin(round_ret_orig_x*new_spacing,
                  round_ret_orig_y*new_spacing,
                  round_ret_orig_z*new_spacing);

  //get origin shift
  algebra::Vector3D orig_shift=ret->get_origin()-in_orig;

  emreal *ret_data=ret->get_data();
  double ret_x_pos, ret_y_pos, ret_z_pos;
  double in_x_ind,in_y_ind,in_z_ind, a, b, c;
  int x0,y0,z0,x1,y1,z1;
  int ret_nyx=ret_ny*ret_nx;
  int in_nyx=in_ny*in_nx;

  for (int iz=0;iz<ret_nz;iz++){
    for (int iy=0;iy<ret_ny;iy++){
      for (int ix=0;ix<ret_nx;ix++){
        //position of ret voxel relative to start of in_map
        ret_x_pos = orig_shift[0] + ix * new_spacing;
        ret_y_pos = orig_shift[1] + iy * new_spacing;
        ret_z_pos = orig_shift[2] + iz * new_spacing;

        //index of ret location in the in_map
        in_x_ind= (ret_x_pos/in_spacing);
        in_y_ind = (ret_y_pos/in_spacing);
        in_z_ind = (ret_z_pos/in_spacing);
        x0 = floor(in_x_ind);
        x1 = ceil(in_x_ind);
        y0 = floor(in_y_ind);
        y1 = ceil(in_y_ind);
        z0 = floor(in_z_ind);
        z1 = ceil(in_z_ind);
        a = in_x_ind-x0;
        b = in_y_ind-y0;
        c = in_z_ind-z0;
        //interpolate
        ret_data[iz*ret_nyx+iy*ret_nx+ix]=
          a * b * c * in_data[z1*in_nyx+y1*in_nx+x1] +
          (1-a) * b * c * in_data[z1*in_nyx+y1*in_nx+x0] +
          a * (1-b) * c * in_data[z1*in_nyx+y0*in_nx+x1] +
          a * b * (1-c) * in_data[z0*in_nyx+y1*in_nx+x1] +
          a * (1-b) * (1-c) * in_data[z0*in_nyx+y0*in_nx+x1] +
          (1-a) * b * (1-c) * in_data[z0*in_nyx+y1*in_nx+x0] +
          (1-a) * (1-b) * c * in_data[z1*in_nyx+y0*in_nx+x0] +
          (1-a) * (1-b) * (1-c) * in_data[z0*in_nyx+y0*in_nx+x0];
      }}}
  return ret.release();
}

algebra::GridD<3,
                     algebra::DenseGridStorageD<3, float>, float >
 get_grid(DensityMap *in) {
  IMP_FUNCTION_LOG;
  IMP_CHECK_OBJECT(in);
  typedef algebra::GridD<3,
                     algebra::DenseGridStorageD<3, float>, float > G;
  G ret(in->get_header()->get_spacing(), get_bounding_box(in));
  IMP_USAGE_CHECK(ret.get_number_of_voxels(0)==
                  static_cast<unsigned int>(in->get_header()->get_nx()),
                  "X voxels don't match");
  IMP_USAGE_CHECK(ret.get_number_of_voxels(1)==
                  static_cast<unsigned int>(in->get_header()->get_ny()),
                  "Y voxels don't match");
  IMP_USAGE_CHECK(ret.get_number_of_voxels(2)==
                  static_cast<unsigned int>(in->get_header()->get_nz()),
                  "Z voxels don't match");
  for (unsigned int i=0; i< ret.get_number_of_voxels(0); ++i) {
    for (unsigned int j=0; j< ret.get_number_of_voxels(1); ++j) {
      for (unsigned int k=0; k< ret.get_number_of_voxels(2); ++k) {
        G::ExtendedIndex ei(i,j,k);
        G::Index gi= ret.get_index(ei);
        long vi= in->get_voxel_by_location(i,j,k);
        ret[gi]= in->get_value(vi);
      }
    }
  }
  return ret;
}


DensityMap* create_density_map(const algebra::GridD<3,
                     algebra::DenseGridStorageD<3, float>,
                                                           float > &arg) {
  IMP_FUNCTION_LOG;
  typedef algebra::GridD<3,
                     algebra::DenseGridStorageD<3, float>, float > G;
  IMP_USAGE_CHECK(std::abs(arg.get_unit_cell()[0]-arg.get_unit_cell()[1]) < .01,
                  "The passed grid does not seem to have cubic voxels");
  Pointer<DensityMap> ret= create_density_map(algebra::get_bounding_box(arg),
                                              arg.get_unit_cell()[0]);
  IMP_USAGE_CHECK(arg.get_number_of_voxels(0)==
                  static_cast<unsigned int>(ret->get_header()->get_nx()),
                  "X voxels don't match");
  IMP_USAGE_CHECK(arg.get_number_of_voxels(1)==
                  static_cast<unsigned int>(ret->get_header()->get_ny()),
                  "Y voxels don't match");
  IMP_USAGE_CHECK(arg.get_number_of_voxels(2)==
                  static_cast<unsigned int>(ret->get_header()->get_nz()),
                  "Z voxels don't match");
  for (unsigned int i=0; i< arg.get_number_of_voxels(0); ++i) {
    for (unsigned int j=0; j< arg.get_number_of_voxels(1); ++j) {
      for (unsigned int k=0; k< arg.get_number_of_voxels(2); ++k) {
        G::ExtendedIndex ei(i,j,k);
        G::Index gi= arg.get_index(ei);
        long vi= ret->get_voxel_by_location(i,j,k);
        ret->set_value(vi, arg[gi]);
      }
    }
  }
  return ret.release();
}

DensityMap* get_binarized_interior(DensityMap *dmap) {
  em::emreal* data = dmap->get_data();
  unsigned int nx,ny,nz;
  nx=dmap->get_header()->get_nx();
  ny=dmap->get_header()->get_ny();
  nz=dmap->get_header()->get_nz();
  Pointer<em::DensityMap> mask_inside = em::create_density_map(dmap);
  mask_inside->set_was_used(true);
  mask_inside->reset_data(0.);
  em::emreal* mdata = mask_inside->get_data();
  int check;
  // mark inside voxels
  for (unsigned int iz=0;iz<nz;iz++) {
    for (unsigned int iy=0;iy<ny;iy++) {
      for (unsigned int ix=0;ix<nx;ix++) {
        long q=ix+(nx)*(iy+ny*iz);
        if (data[q]>0.0) { //inside or on the surface
          mdata[q]=1;
        } else { //check all directions
          check=0;
          for(unsigned int i=0;i<ix;i++)
            if (data[i+(nx)*(iy+ny*iz)]>0.0) {
              check++; i=ix;
            }
          for(unsigned int i=ix+1;i<nx;i++)
            if (data[i+(nx)*(iy+ny*iz)]>0.0) {
              check++; i=nx;
            }
          for(unsigned int i=0;i<iy;i++)
            if (data[ix+(nx)*(i+ny*iz)]>0.0) {
              check++; i=iy;
            }
          for(unsigned int i=iy;i<ny;i++)
            if (data[ix+(nx)*(i+ny*iz)]>0.0) {
              check++; i=ny;
            }
          for(unsigned int i=0;i<iz;i++)
            if (data[ix+(nx)*(iy+ny*i)]>0.0) {
              check++; i=iz;
            }
          for(unsigned int i=iz;i<nz;i++)
            if (data[ix+(nx)*(iy+ny*i)]>0.0) {
              check++; i=nz;
            }
          if (check>=4) { //inside
            mdata[q]=1;
          }
        }
      }
    }
  }
  //remove surface
  Pointer<em::DensityMap> mask_inside2 = em::create_density_map(mask_inside);
  mask_inside2->set_was_used(true);
  em::emreal* mdata2 = mask_inside2->get_data();
  long ind2;
  int shell_w=1;
  for (int iz=0;iz<(int)nz;iz++) {
    for (int iy=0;iy<(int)ny;iy++) {
      for (int ix=0;ix<(int)nx;ix++) {
        long ind=ix+(nx)*(iy+ny*iz);
        if (mdata[ind]==1) {
          check=0;
          for (int iz2=-shell_w;check==0&&iz2<=shell_w;iz2++)
            for (int iy2=-shell_w;check==0&&iy2<=shell_w;iy2++)
              for (int ix2=-shell_w;check==0&&ix2<=shell_w;ix2++) {
                if (ix+ix2 < (int)nx &&
                    iy+iy2 < (int)ny &&
                    iz+iz2 < (int)nz &&
                    ix+ix2 >= 0 && iy+iy2 >= 0 && iz+iz2 >= 0) {
                  ind2=(ix+ix2)+(nx)*((iy+iy2)+ny*(iz+iz2));
                  if (mdata[ind2]==0) check=1; //surface point
                } else check=2; /* out of bounds */
              }
          if ((check > 0)&&(mdata2[ind] == 1)) {mdata2[ind]=0;}
        }
      }
    }
  }
  return mask_inside2.release();
}
IMPEM_END_NAMESPACE
