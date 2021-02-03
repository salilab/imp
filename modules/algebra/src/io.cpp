/**
 *  \file io.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/io.h>

IMPALGEBRA_BEGIN_NAMESPACE

void write_pts(const Vector3Ds &vs, TextOutput out) {
  for (unsigned int i = 0; i < vs.size(); ++i) {
    out.get_stream() << spaces_io(vs[i]) << "\n";
  }
}

Vector3Ds read_pts(TextInput oin) {
  std::istream &in = oin;
  Vector3Ds ret;
  while (true) {
    char buf[2000];
    in.getline(buf, 2000);
    if (!in) break;
    ;
    if (buf[0] == '#') continue;
    std::istringstream iss(buf);
    IMP::Float x, y, z;
    iss >> x >> y >> z;
    if (!iss) {
      throw IMP::ValueException(
          (std::string("Unable to parse line ") + buf).c_str());
    }
    Vector3D v(x, y, z);
    ret.push_back(v);
  }
  return ret;
}

void write_spheres(const Sphere3Ds &vs, TextOutput out) {
  for (unsigned int i = 0; i < vs.size(); ++i) {
    out.get_stream() << spaces_io(vs[i]) << "\n";
  }
}

Sphere3Ds read_spheres(TextInput oin) {
  Sphere3Ds ret;
  std::istream &in = oin;
  while (true) {
    char buf[2000];
    in.getline(buf, 2000);
    if (!in) break;
    ;
    if (buf[0] == '#') continue;
    std::istringstream iss(buf);
    IMP::Float x, y, z, r;
    iss >> x >> y >> z >> r;
    if (!iss) {
      throw IMP::ValueException(
          (std::string("Unable to parse line ") + buf).c_str());
    }
    Sphere3D v(Vector3D(x, y, z), r);
    ret.push_back(v);
  }
  return ret;
}

IMPALGEBRA_END_NAMESPACE
