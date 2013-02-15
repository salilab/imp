/**
 *  \file writers.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/display/internal/utility.h>
#include <IMP/display/declare_Geometry.h>
#include <IMP/display/primitive_geometries.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/warning_macros.h>
#include <algorithm>
#ifdef IMP_DISPLAY_USE_IMP_CGAL
#include <IMP/cgal/internal/polygons.h>
#endif

IMPDISPLAY_BEGIN_INTERNAL_NAMESPACE

namespace {
#ifdef IMP_DISPLAY_USE_IMP_CGAL
  Ints get_fan_triangles(Ints::const_iterator b,
                         Ints::const_iterator e) {
    IMP_INTERNAL_CHECK(std::distance(b,e)>= 3, "Too few vertices");
    Ints ret((std::distance(b,e)-2)*3, -1);
    for (unsigned int i=0; i< ret.size()/3; ++i) {
      ret.at(i*3)= *b;
      ret.at(i*3+1)= *(b+i+1);
      ret.at(i*3+2)= *(b+i+2);
    }
    IMP_INTERNAL_CHECK(std::find(ret.begin(), ret.end(), -1)
                       == ret.end(), "Still -1 found");
    return ret;
  }

  Ints get_triangulation_of_convex_faces(const Ints &faces) {
    IMP_LOG_VERBOSE( "Convex faces are " << faces << std::endl);
    Ints ret;
    Ints::const_iterator it= faces.begin();
    while (it != faces.end()) {
      Ints::const_iterator eit
        = std::find(it,
                    static_cast<Ints::const_iterator>(faces.end()),
                    -1);
      IMP_INTERNAL_CHECK(eit != faces.end(),
                         "No trailing -1");
      Ints tris= get_fan_triangles(it, eit);
      ret.insert(ret.end(), tris.begin(), tris.end());
      it=eit;
      ++it;
    }
    IMP_LOG_VERBOSE( "Triangles are " << ret << std::endl);
    return ret;
  }
#endif
  Ints get_triangulation_of_face(const Ints &face,
                                 const algebra::Vector3Ds &vertices) {
    IMP_UNUSED(vertices[0]);
    if (face.size()==3) {
      return face;
    } else {
      IMP_LOG_VERBOSE( "Non-convex face are " << face << std::endl);
#ifdef IMP_DISPLAY_USE_IMP_CGAL
      Ints curb= IMP::cgal::internal::get_convex_polygons(face,
                                       vertices);
      Ints ret= get_triangulation_of_convex_faces(curb);
      IMP_LOG_VERBOSE( "Triangles are " << ret << std::endl);
      return ret;
#else
      IMP_UNUSED(face.size());
      IMP_THROW("Non-triangular face found. Processing those requires CGAL.",
                base::ValueException);
#endif
    }
  }
}

Ints get_triangles(SurfaceMeshGeometry *sg) {
  Ints ret;
  Ints::const_iterator it= sg->get_faces().begin();
  while (it != sg->get_faces().end()) {
    Ints::const_iterator eit
    = std::find(it,
                static_cast<Ints::const_iterator>(sg->get_faces().end()),
                -1);
    IMP_INTERNAL_CHECK(eit != sg->get_faces().end(),
                       "No trailing -1");
    Ints cur(it, eit);
    Ints tris= get_triangulation_of_face(cur,
                                         sg->get_vertexes());
    ret.insert(ret.end(), tris.begin(), tris.end());
    it=eit;
    ++it;
  }
  return ret;
}


Ints get_triangles(PolygonGeometry *sg) {
  Ints indexes(sg->get_geometry().size());
  for (unsigned int i=0; i< indexes.size(); ++i) {
    indexes[i]=i;
  }
  return get_triangulation_of_face(indexes, sg->get_geometry());
}

namespace {
  template <class It>
  algebra::Vector3D get_normal(It b, It e, const algebra::Vector3Ds &vertices) {
    IMP_CHECK_VARIABLE(e);
    IMP_USAGE_CHECK(std::distance(b,e)==3, "Not a triangle");
    algebra::Vector3D n=
      get_vector_product(vertices.at(*(b+1))-vertices.at(*b),
                         vertices.at(*(b+2))-vertices.at(*b)).get_unit_vector();
    return n;
  }
}


algebra::Vector3Ds get_normals(const Ints &faces,
                               const algebra::Vector3Ds &vertices) {
  IMP_INTERNAL_CHECK(faces.size()%3==0, "Not triangles");
  IMP_INTERNAL_CHECK(faces.size()>0, "No faces");
  Ints count(vertices.size());
  algebra::Vector3Ds sum(vertices.size(), algebra::get_zero_vector_d<3>());
  for (unsigned int i=0; i< faces.size()/3; ++i) {
    algebra::Vector3D n= get_normal(faces.begin()+3*i,
                                    faces.begin()+3*i+3,
                                    vertices);
    IMP_INTERNAL_CHECK(!base::isnan(n[0]), "Nan found");
    for (unsigned int j=0; j< 3; ++j) {
      int v=faces[3*i+j];
      sum[v]+= n;
      count[v]+=1;
    }
  }
  for (unsigned int i=0; i< count.size(); ++i) {
    sum[i]/=count[i];
    IMP_INTERNAL_CHECK(!base::isnan(sum[i][0]), "Nan found at end:"
                       << count[i]<< " on " << i);
  }
  return sum;
}


IMPDISPLAY_END_INTERNAL_NAMESPACE
