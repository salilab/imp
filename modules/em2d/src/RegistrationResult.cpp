/**
 *  \file RegistrationResult.cpp
 *  \brief Registration results class
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/internal/rotation_helper.h"

IMPEM2D_BEGIN_NAMESPACE

RegistrationResult::~RegistrationResult() {};



RegistrationResults get_random_registration_results(unsigned long n,
                                double maximum_shift) {
  srand (time(NULL));
  RegistrationResults results;
  for (unsigned long i=0;i<n;++i) {
    RegistrationResult rr;
    rr.set_random_registration(i,maximum_shift);
    results.push_back(rr);
  }
  return results;
}




RegistrationResults evenly_distributed_registration_results(
                                          unsigned long n_projections) {
  algebra::SphericalVector3Ds vs;
  em2d::internal::semispherical_even_distribution(n_projections,vs);
  RegistrationResults results;
  for (unsigned long i=0;i<n_projections;++i) {
    algebra::Rotation3D R=
         em2d::internal::get_rotation_from_projection_direction(vs[i]);
    algebra::Vector2D shift(0.0,0.0);
    results.push_back(RegistrationResult(R,shift,i) );
  }
  return results;
}


void  write_registration_results(String filename, RegistrationResults results) {
  std::ofstream f(filename.c_str(),std::ios::out | std::ios::binary);
  f << results.size() << std::endl;
  for (unsigned long i=0;i<results.size();++i) {
    results[i].write(f);
  }
  f.close();
}




void RegistrationResult::set_random_registration(unsigned long index,
                                double maximum_shift) {
  // Random point in the sphere, pick to ensure even distribution
  double u=random_between_zero_and_one();
  double v=random_between_zero_and_one();
  double w=random_between_zero_and_one();
  double phi  =  2*PI*u;
  double theta  =  acos((2*v-1))/2;
  double psi=2*PI*w;
  set_rotation(phi,theta,psi);
  shift_[0] =  maximum_shift*random_between_zero_and_one();
  shift_[1] =  maximum_shift*random_between_zero_and_one();
  ccc_=0.0;
  String name_="";
  index_=index;
}

void RegistrationResult::read_from_image(em::Image &im) {
  im.get_header().get_euler_angles(phi_,theta_,psi_);
  im.get_header().get_origin_offsets(shift_[0],shift_[1]);
  set_rotation(phi_,theta_,psi_);
}


void RegistrationResult::set_in_image(em::Image &im) {
  im.get_header().set_euler_angles(phi_,theta_,psi_);
  im.get_header().set_origin_offsets(shift_);
  set_rotation(phi_,theta_,psi_);
}

  void RegistrationResult::set_rotation(algebra::Rotation3D &R){
    this->R_=R;
    algebra::Vector3D angles=
                    em2d::internal::get_euler_angles_from_rotation(R,3,2);
    this->phi_ = angles[0];
    this->theta_=angles[1];
    this->psi_=angles[2];
  }


void RegistrationResult::read(const String &line) {
  algebra::VectorD<4> quaternion;
  String s = line;
  size_t n;
  n=s.find("|");
  index_=std::atoi(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  phi_ =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  theta_ =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  psi_ =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  quaternion[0] =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  quaternion[1] =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  quaternion[2] =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  quaternion[3] =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  shift_[0] =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  shift_[1] =std::atof(s.substr(0,n).c_str());
  s=s.substr(n+1); n=s.find("|");
  ccc_ =std::atof(s.substr(0,n).c_str());
  set_rotation(phi_,theta_,psi_);
}

//! Reads a set of registration results
RegistrationResults read_registration_results(String filename) {
  std::ifstream in(filename.c_str(),std::ios::in | std::ios::binary);
  unsigned int n_records=0;
  String line;
  while(!in.eof()) {
    getline(in,line);
    if(line[0]=='#') {
      continue;
    } else {
      n_records=std::atoi(line.c_str());
      break;
    }
  }
  RegistrationResults results(n_records);
  for(unsigned int i=0;i<n_records;++i) {
    getline(in,line);
    results[i].read(line);
  }
  in.close();
  return results;
}




bool has_higher_ccc(const RegistrationResult &rr1,
                                  const RegistrationResult &rr2) {
  if(rr1.get_ccc() >= rr2.get_ccc()) return true;
  return false;
}

IMPEM2D_END_NAMESPACE
