/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/cgal/internal/polyhedrons.h>
#include <CGAL/Origin.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Handle_hash_function.h>
#include <IMP/internal/map.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/make_skin_surface_mesh_3.h>
#include <CGAL/Union_of_balls_3.h>
#include <CGAL/Gmpq.h>


namespace {
  typedef CGAL::Exact_predicates_exact_constructions_kernel EKernel;
  typedef CGAL::Exact_predicates_inexact_constructions_kernel IKernel;
}
namespace CGAL {
  namespace internal {
    unsigned int hash_value( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
    bool operator<( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1) {
      return &*vh0 < &*vh1;
    }
  }
  // older CGAL
  namespace CGALi {
    unsigned int hash_value( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
    bool operator<( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1) {
      return &*vh0 < &*vh1;
    }
  }
  namespace internal {
    unsigned int hash_value( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
    bool operator<( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<IKernel>::Vertex_handle vh1) {
      return &*vh0 < &*vh1;
    }
  }
  // older CGAL
  namespace CGALi {
    unsigned int hash_value( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
    bool operator<( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<IKernel>::Vertex_handle vh1) {
      return &*vh0 < &*vh1;
    }
  }
}

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
// /Users/drussel/fs/include/CGAL/Nef_3/Halfedge.h:110:
// error: 'ORIGIN' is not a member of 'CGAL'

namespace {

  template <class K>
  algebra::VectorD<3> tr(const typename K::Point_3 pt) {
    return algebra::VectorD<3>(CGAL::to_double(pt[0]),
                               CGAL::to_double(pt[1]),
                               CGAL::to_double(pt[2]));
  }
  template <class K>
  typename K::Point_3 tr(const algebra::VectorD<3> pt) {
    return typename K::Point_3(pt[0], pt[1], pt[2]);
  }
  template <class K>
  typename K::Plane_3 tr(const algebra::Plane3D &p) {
    return typename K::Plane_3(p.get_normal()[0],
                   p.get_normal()[1],
                   p.get_normal()[2],
                   -p.get_distance_from_origin());
  }

  template <class K>
  std::vector<std::vector<algebra::VectorD<3> > >
  get_facets(  CGAL::Nef_polyhedron_3<K> &np) {
    typename CGAL::Polyhedron_3<K> p;
    np.convert_to_polyhedron(p);
    CGAL_postcondition( p.is_valid());
    typename std::vector<typename
                         std::vector<typename algebra::VectorD<3> > > ret;
    for (typename CGAL::Polyhedron_3<K>::Face_iterator it= p.facets_begin();
         it != p.facets_end(); ++it) {
      ret.push_back(typename std::vector<typename algebra::VectorD<3> >());
      typename CGAL::Polyhedron_3<K>
        ::Facet::Halfedge_around_facet_circulator c= it->facet_begin();
      do {
        ret.back().push_back(tr<K>(c->vertex()->point()));
        ++c;
      } while (c != it->facet_begin());
    }
    return ret;
  }


  template <class K>
  std::pair<std::vector<algebra::VectorD<3> >,std::vector<Ints> >
  get_indexed_facets(  CGAL::Polyhedron_3<K> &p) {
    CGAL_postcondition( p.is_valid());
    std::vector<Ints> faces;
    std::vector<algebra::VectorD<3> > coords;
    typename IMP::internal::Map<typename CGAL::Polyhedron_3<K>::Vertex_handle,
                                int> vertices;
    //std::map<Polyhedron::Vertex_handle, int> vertices;
    for (typename CGAL::Polyhedron_3<K>::Face_iterator it= p.facets_begin();
         it != p.facets_end(); ++it) {
      faces.push_back(Ints());
      typename CGAL::Polyhedron_3<K>::Facet
        ::Halfedge_around_facet_circulator c= it->facet_begin();
      do {
        typename CGAL::Polyhedron_3<K>::Vertex_handle vh= c->vertex();
        if (vertices.find(vh) == vertices.end()) {
          vertices[vh]= coords.size();
          coords.push_back(tr<K>(vh->point()));
        }
        faces.back().push_back(vertices.find(vh)->second);
        ++c;
      } while (c != it->facet_begin());
    }
    return std::make_pair(coords, faces);
  }

  template <class K>
  std::pair<std::vector<algebra::VectorD<3> >,std::vector<Ints> >
  get_indexed_facets(typename CGAL::Nef_polyhedron_3<K> &np) {
    typename CGAL::Polyhedron_3<K> p;
    np.convert_to_polyhedron(p);
    return get_indexed_facets(p);
  }

  template <class K>
  CGAL::Nef_polyhedron_3<K> create_cube(const algebra::BoundingBoxD<3> &bb) {
    typename CGAL::Polyhedron_3<K> p;
    // appends a cube of size [0,1]^3 to the polyhedron P.
    CGAL_precondition( p.is_valid());
    typedef typename CGAL::Polyhedron_3<K>::Halfedge_handle Halfedge_handle;
    Halfedge_handle h
      = p.make_tetrahedron( typename K::Point_3( bb.get_corner(1)[0],
                                                 bb.get_corner(0)[1],
                                                 bb.get_corner(0)[2]),
                            typename K::Point_3( bb.get_corner(0)[0],
                                                 bb.get_corner(0)[1],
                                                 bb.get_corner(1)[2]),
                            typename K::Point_3(bb.get_corner(0)[0],
                                                bb.get_corner(0)[1],
                                                bb.get_corner(0)[2]),
                            typename K::Point_3( bb.get_corner(0)[0],
                                                 bb.get_corner(1)[1],
                                                 bb.get_corner(0)[2]));
    Halfedge_handle g = h->next()->opposite()->next();             // Fig. (a)
    p.split_edge( h->next());
    p.split_edge( g->next());
    p.split_edge( g);                                              // Fig. (b)
    h->next()->vertex()->point()
      = typename K::Point_3( bb.get_corner(1)[0],
                             bb.get_corner(0)[1],
                             bb.get_corner(1)[2]);
    g->next()->vertex()->point()
      = typename K::Point_3( bb.get_corner(0)[0],
                             bb.get_corner(1)[1],
                             bb.get_corner(1)[2]);
    g->opposite()->vertex()->point()
      = typename K::Point_3( bb.get_corner(1)[0],
                             bb.get_corner(1)[1],
                             bb.get_corner(0)[2]);
    // Fig. (c)
    Halfedge_handle f = p.split_facet( g->next(),
                                       g->next()->next()->next()); // Fig. (d)
    Halfedge_handle e = p.split_edge( f);
    e->vertex()->point()
      = typename K::Point_3( bb.get_corner(1)[0],
                             bb.get_corner(1)[1],
                             bb.get_corner(1)[2]);          // Fig. (e)
    p.split_facet( e, f->next()->next());                          // Fig. (f)
    CGAL_postcondition( p.is_valid());
    return p;
  }

  template <class K>
  CGAL::Nef_polyhedron_3<K> create_nef(const algebra::BoundingBoxD<3> &bb,
                            const std::vector< algebra::Plane3D >&planes) {
    typename CGAL::Nef_polyhedron_3<K> cur(create_cube<K>(bb));
    IMP_INTERNAL_CHECK(cur.is_simple(), "Something wrong with cube ");
    for (unsigned int i=0; i< planes.size(); ++i) {
      cur= cur.intersection(tr<K>(planes[i]),
                            CGAL::Nef_polyhedron_3<K>::CLOSED_HALFSPACE);
      typename CGAL::Polyhedron_3<K> p;
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
  CGAL::Nef_polyhedron_3<EKernel> pouter= create_nef<EKernel>(bb, outer);
  CGAL::Nef_polyhedron_3<EKernel> phole= create_nef<EKernel>(bb, hole);
  CGAL::Nef_polyhedron_3<EKernel> diff= pouter-phole;
  return get_facets(diff);
}

std::pair<std::vector<algebra::VectorD<3> >,std::vector<Ints> >
get_polyhedron_indexed_facets(const algebra::BoundingBoxD<3> &bb,
                              const std::vector< algebra::Plane3D > &outer,
                              const std::vector< algebra::Plane3D > &hole) {
  CGAL::Nef_polyhedron_3<EKernel> pouter= create_nef<EKernel>(bb, outer);
  CGAL::Nef_polyhedron_3<EKernel> phole= create_nef<EKernel>(bb, hole);
  CGAL::Nef_polyhedron_3<EKernel> diff= pouter-phole;
  return get_indexed_facets(diff);
}


std::vector<std::vector<algebra::VectorD<3> > >
get_polyhedron_facets(const algebra::BoundingBoxD<3> &bb,
                      const std::vector< algebra::Plane3D > &outer) {
  CGAL::Nef_polyhedron_3<EKernel> pouter= create_nef<EKernel>(bb, outer);
  return get_facets(pouter);
}



std::pair<std::vector<algebra::VectorD<3> >,std::vector<Ints> >
get_skin_surface(const std::vector<algebra::SphereD<3> > &ss) {
  typedef IKernel::Point_3                                     Bare_point;
  typedef CGAL::Weighted_point<Bare_point,IKernel::RT>         Weighted_point;
  std::vector<Weighted_point> l(ss.size());
  for (unsigned int i=0; i< ss.size(); ++i) {
    l[i]= Weighted_point(tr<IKernel>(ss[i].get_center()),
                         (ss[i].get_radius()));
  }
  CGAL::Polyhedron_3<IKernel> p;
  CGAL::Union_of_balls_3<CGAL::Skin_surface_traits_3<IKernel> >
    skin_surface(l.begin(), l.end());
  CGAL::mesh_skin_surface_3(skin_surface, p);
  //CGAL::make_skin_surface_mesh_3(p, l.begin(), l.end(), 1.0);
  return get_indexed_facets(p);
}


IMPCGAL_END_INTERNAL_NAMESPACE
