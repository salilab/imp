/**
 *  \file io.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/algebra/io.h>


IMPALGEBRA_BEGIN_NAMESPACE

void write_pts(const std::vector<VectorD<3> > &vs, TextOutput out) {
  for (unsigned int i=0; i< vs.size(); ++i) {
    out.get_stream() << spaces_io(vs[i]) << "\n";
  }
}

std::vector<VectorD<3> > read_pts(TextInput oin) {
  std::istream &in= oin;
  std::vector<VectorD<3> > ret;
  while (true) {
    char buf[2000];
    in.getline(buf, 2000);
    if (!in) break;;
    if (buf[0]=='#') continue;
    std::istringstream iss(buf);
    IMP::Float x,y,z;
    iss >> x >> y >> z;
    if (!iss) {
      throw IMP::ValueException((std::string("Unable to parse line ")
                                 + buf).c_str());
    }
    VectorD<3> v(x,y,z);
    ret.push_back(v);
  }
  return ret;
}

void write_spheres(const std::vector<VectorD<3> > &vs, TextOutput out) {
  for (unsigned int i=0; i< vs.size(); ++i) {
    out.get_stream() << spaces_io(vs[i]) << "\n";
  }
}

std::vector<SphereD<3> > read_spheres(TextInput oin) {
  std::vector<SphereD<3> > ret;
  std::istream &in=oin;
  while (true) {
    char buf[2000];
    in.getline(buf, 2000);
    if (!in) break;;
    if (buf[0]=='#') continue;
    std::istringstream iss(buf);
    IMP::Float x,y,z, r;
    iss >> x >> y >> z >> r;
    if (!iss) {
      throw IMP::ValueException((std::string("Unable to parse line ")
                                 + buf).c_str());
    }
    SphereD<3> v(VectorD<3>(x,y,z), r);
    ret.push_back(v);
  }
  return ret;
}


IMPALGEBRA_END_NAMESPACE
