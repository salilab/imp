/**
 *  \file io.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/io.h>


IMPALGEBRA_BEGIN_NAMESPACE

void write_pts(const std::vector<Vector3D > &vs, TextOutput out) {
  for (unsigned int i=0; i< vs.size(); ++i) {
    out.get_stream() << spaces_io(vs[i]) << "\n";
  }
}

std::vector<Vector3D > read_pts(TextInput oin) {
  std::istream &in= oin;
  std::vector<Vector3D > ret;
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
    Vector3D v(x,y,z);
    ret.push_back(v);
  }
  return ret;
}

void write_spheres(const std::vector<Sphere3D > &vs,
                   TextOutput out) {
  for (unsigned int i=0; i< vs.size(); ++i) {
    out.get_stream() << spaces_io(vs[i]) << "\n";
  }
}

std::vector<Sphere3D > read_spheres(TextInput oin) {
  std::vector<Sphere3D > ret;
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
    Sphere3D v(Vector3D(x,y,z), r);
    ret.push_back(v);
  }
  return ret;
}


IMPALGEBRA_END_NAMESPACE
