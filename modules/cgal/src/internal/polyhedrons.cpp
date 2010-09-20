/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include <IMP/cgal/internal/polyhedrons.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Handle_hash_function.h>
#include <IMP/internal/map.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Gmpq.h>


namespace {
  typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
  typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;
  typedef Nef_polyhedron::Plane_3 Plane_3;
  typedef Nef_polyhedron::Vector_3 Vector_3;

  typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
  typedef Kernel::Point_3 Point_3;
}
namespace CGAL {
  namespace internal {
    unsigned int hash_value(Polyhedron::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
  }
  // older CGAL
  namespace CGALi {
    unsigned int hash_value(Polyhedron::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
  }
}

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
// /Users/drussel/fs/include/CGAL/Nef_3/Halfedge.h:110:
// error: 'ORIGIN' is not a member of 'CGAL'

namespace {

  algebra::VectorD<3> tr(const Point_3 pt) {
    return algebra::VectorD<3>(CGAL::to_double(pt[0]),
                               CGAL::to_double(pt[1]),
                               CGAL::to_double(pt[2]));
  }
  Plane_3 tr(const algebra::Plane3D &p) {
    return Plane_3(p.get_normal()[0],
                   p.get_normal()[1],
                   p.get_normal()[2],
                   -p.get_distance_from_origin());
  }
  /*
/Users/drussel/src/IMP/svn/svn/modules/cgal/src/internal/polyhedrons.cpp:41:
error: passing 'const IMP::cgal::internal::<unnamed>::Nef_polyhedron'
as 'this' argument of 'void CGAL::Nef_polyhedron_3<K, I,
Mk>::convert_to_polyhedron(Polyhedron&) [with Polyhedron =
IMP::cgal::internal::<unnamed>::Polyhedron, Kernel_ =
IMP::cgal::internal::<unnamed>::Kernel, Items_ =
CGAL::SNC_indexed_items, Mark_ = bool]' discards qualifiers
   */
std::vector<std::vector<algebra::VectorD<3> > >
get_facets(  Nef_polyhedron &np) {
  Polyhedron p;
  np.convert_to_polyhedron(p);
  CGAL_postcondition( p.is_valid());
  std::vector<std::vector<algebra::VectorD<3> > > ret;
  for (Polyhedron::Face_iterator it= p.facets_begin();
       it != p.facets_end(); ++it) {
    ret.push_back(std::vector<algebra::VectorD<3> >());
    Polyhedron::Facet::Halfedge_around_facet_circulator c= it->facet_begin();
    do {
      ret.back().push_back(tr(c->vertex()->point()));
      ++c;
    } while (c != it->facet_begin());
  }
  return ret;
}


std::pair<std::vector<algebra::VectorD<3> >,std::vector<Ints> >
get_indexed_facets(  Nef_polyhedron &np) {
  Polyhedron p;
  np.convert_to_polyhedron(p);
  CGAL_postcondition( p.is_valid());
  std::vector<Ints> faces;
  std::vector<algebra::VectorD<3> > coords;
  IMP::internal::Map<Polyhedron::Vertex_handle, int> vertices;
  for (Polyhedron::Face_iterator it= p.facets_begin();
       it != p.facets_end(); ++it) {
    faces.push_back(Ints());
    Polyhedron::Facet::Halfedge_around_facet_circulator c= it->facet_begin();
    do {
      Polyhedron::Vertex_handle vh= c->vertex();
      if (vertices.find(vh) == vertices.end()) {
        vertices[vh]= coords.size();
        coords.push_back(tr(vh->point()));
      }
      faces.back().push_back(vertices.find(vh)->second);
      ++c;
    } while (c != it->facet_begin());
  }
  return std::make_pair(coords, faces);
}

  Nef_polyhedron create_cube(const algebra::BoundingBoxD<3> &bb) {
    Polyhedron p;
    // appends a cube of size [0,1]^3 to the polyhedron P.
    CGAL_precondition( p.is_valid());
    typedef Polyhedron::Halfedge_handle Halfedge_handle;
    Halfedge_handle h = p.make_tetrahedron( Point_3( bb.get_corner(1)[0],
                                                     bb.get_corner(0)[1],
                                                     bb.get_corner(0)[2]),
                                            Point_3( bb.get_corner(0)[0],
                                                     bb.get_corner(0)[1],
                                                     bb.get_corner(1)[2]),
                                            Point_3(bb.get_corner(0)[0],
                                                    bb.get_corner(0)[1],
                                                    bb.get_corner(0)[2]),
                                            Point_3( bb.get_corner(0)[0],
                                                     bb.get_corner(1)[1],
                                                     bb.get_corner(0)[2]));
    Halfedge_handle g = h->next()->opposite()->next();             // Fig. (a)
    p.split_edge( h->next());
    p.split_edge( g->next());
    p.split_edge( g);                                              // Fig. (b)
    h->next()->vertex()->point()     = Point_3( bb.get_corner(1)[0],
                                              bb.get_corner(0)[1],
                                              bb.get_corner(1)[2]);
    g->next()->vertex()->point()     = Point_3( bb.get_corner(0)[0],
                                              bb.get_corner(1)[1],
                                              bb.get_corner(1)[2]);
    g->opposite()->vertex()->point() = Point_3( bb.get_corner(1)[0],
                                                bb.get_corner(1)[1],
                                                bb.get_corner(0)[2]);
    // Fig. (c)
    Halfedge_handle f = p.split_facet( g->next(),
                                       g->next()->next()->next()); // Fig. (d)
    Halfedge_handle e = p.split_edge( f);
    e->vertex()->point() = Point_3( bb.get_corner(1)[0],
                                    bb.get_corner(1)[1],
                                    bb.get_corner(1)[2]);          // Fig. (e)
    p.split_facet( e, f->next()->next());                          // Fig. (f)
    CGAL_postcondition( p.is_valid());
    return p;
}

  Nef_polyhedron create_nef(const algebra::BoundingBoxD<3> &bb,
                            const std::vector< algebra::Plane3D >&planes) {
    Nef_polyhedron cur(create_cube(bb));
    IMP_INTERNAL_CHECK(cur.is_simple(), "Something wrong with cube ");
    for (unsigned int i=0; i< planes.size(); ++i) {
      cur= cur.intersection(tr(planes[i]), Nef_polyhedron::CLOSED_HALFSPACE);
      Polyhedron p;
      try {
        cur.convert_to_polyhedron(p);
        //std::cout << p << std::endl;
      } catch(...){
        IMP_INTERNAL_CHECK(false, "Error after intersection with " << i
                           << " which is " << planes[i]);
      }
    }
    IMP_INTERNAL_CHECK(cur.is_simple(), "Something wrong with cut cube ");
    return cur;
  }
}

std::vector<std::vector<algebra::VectorD<3> > >
get_polyhedron_facets(const algebra::BoundingBoxD<3> &bb,
                      const std::vector< algebra::Plane3D > &outer,
                      const std::vector< algebra::Plane3D > &hole) {
  Nef_polyhedron pouter= create_nef(bb, outer);
  Nef_polyhedron phole= create_nef(bb, hole);
  Nef_polyhedron diff= pouter-phole;
  return get_facets(diff);
}

std::pair<std::vector<algebra::VectorD<3> >,std::vector<Ints> >
get_polyhedron_indexed_facets(const algebra::BoundingBoxD<3> &bb,
                      const std::vector< algebra::Plane3D > &outer,
                      const std::vector< algebra::Plane3D > &hole) {
  Nef_polyhedron pouter= create_nef(bb, outer);
  Nef_polyhedron phole= create_nef(bb, hole);
  Nef_polyhedron diff= pouter-phole;
  return get_indexed_facets(diff);
}


std::vector<std::vector<algebra::VectorD<3> > >
get_polyhedron_facets(const algebra::BoundingBoxD<3> &bb,
                      const std::vector< algebra::Plane3D > &outer) {
  Nef_polyhedron pouter= create_nef(bb, outer);
  return get_facets(pouter);
}


IMPCGAL_END_INTERNAL_NAMESPACE
