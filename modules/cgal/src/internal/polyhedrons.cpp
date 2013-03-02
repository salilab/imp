/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/cgal/internal/polyhedrons.h>
#include <IMP/base/log.h>
#include <IMP/algebra/grid_utility.h>
IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
#include <CGAL/Origin.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Handle_hash_function.h>
#include <IMP/base/map.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/make_skin_surface_mesh_3.h>
#include <CGAL/Union_of_balls_3.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Surface_mesh_simplification/HalfedgeGraph_Polyhedron_3.h>
/*#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/
Edge_collapse/Count_stop_predicate.h>
*/
#include <CGAL/IO/output_surface_facets_to_polyhedron.h>


namespace {
  typedef CGAL::Exact_predicates_exact_constructions_kernel EKernel;
  typedef CGAL::Exact_predicates_inexact_constructions_kernel IKernel;
}
namespace CGAL {
  namespace internal {
    unsigned int hash_value( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh);
    unsigned int hash_value( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
    bool operator<( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1);
    bool operator<( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1) {
      return &*vh0 < &*vh1;
    }
  }
  // older CGAL
  namespace CGALi {
    unsigned int hash_value( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh);
    unsigned int hash_value( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh) {
      return CGAL::Handle_hash_function()(vh);
    }
    bool operator<( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1);
    bool operator<( CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1) {
      return &*vh0 < &*vh1;
    }
  }
  namespace internal {
    unsigned int hash_value( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh) ;
    bool operator<( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<IKernel>::Vertex_handle vh1);
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
    unsigned int hash_value( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh);
    bool operator<( CGAL::Polyhedron_3<IKernel>::Vertex_handle vh0,
                    CGAL::Polyhedron_3<IKernel>::Vertex_handle vh1);

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
namespace {

  struct AddressLess {
    template <class T>
    bool operator()(const T&a, const T&b) const {
      return &*a < &*b;
    }
  };

  template <class K>
  algebra::Vector3D trp(const typename CGAL::Point_3<K> pt) {
    return algebra::Vector3D(CGAL::to_double(pt[0]),
                               CGAL::to_double(pt[1]),
                               CGAL::to_double(pt[2]));
  }
  template <class K>
  typename K::Point_3 trp(const algebra::Vector3D pt) {
    return typename K::Point_3(pt[0], pt[1], pt[2]);
  }
  template <class K>
  typename K::Plane_3 trp(const algebra::Plane3D &p) {
    return typename K::Plane_3(p.get_normal()[0],
                               p.get_normal()[1],
                               p.get_normal()[2],
                               -p.get_distance_from_origin());
  }

  template <class K>
  base::Vector<algebra::Vector3Ds >
  get_facets(  CGAL::Nef_polyhedron_3<K> &np) {
    typename CGAL::Polyhedron_3<K> p;
    np.convert_to_polyhedron(p);
    CGAL_postcondition( p.is_valid());
    base::Vector< base::Vector<typename algebra::Vector3D > > ret;
    for (typename CGAL::Polyhedron_3<K>::Face_iterator it= p.facets_begin();
         it != p.facets_end(); ++it) {
      ret.push_back( base::Vector< algebra::Vector3D >());
      typename CGAL::Polyhedron_3<K>
        ::Facet::Halfedge_around_facet_circulator c= it->facet_begin();
      do {
        ret.back().push_back(trp<K>(c->vertex()->point()));
        ++c;
      } while (c != it->facet_begin());
    }
    return ret;
  }


  template <class K>
  std::pair<algebra::Vector3Ds,Ints >
  get_indexed_facets(  CGAL::Polyhedron_3<K> &p) {
    //CGAL_precondition( p.is_valid(true));
    Ints faces;
    algebra::Vector3Ds coords;
    typename std::map<typename CGAL::Polyhedron_3<K>::Vertex_handle,
                      int, AddressLess> vertices;
    //std::map<Polyhedron::Vertex_handle, int> vertices;
    for (typename CGAL::Polyhedron_3<K>::Face_iterator it= p.facets_begin();
         it != p.facets_end(); ++it) {
      typename CGAL::Polyhedron_3<K>::Facet
        ::Halfedge_around_facet_circulator c= it->facet_begin();
      do {
        typename CGAL::Polyhedron_3<K>::Vertex_handle vh= c->vertex();
        if (vertices.find(vh) == vertices.end()) {
          vertices[vh]= coords.size();
          coords.push_back(trp(vh->point()));
        }
        faces.push_back(vertices.find(vh)->second);
        ++c;
      } while (c != it->facet_begin());
      if (!faces.empty() && faces.back()!=-1) {
        faces.push_back(-1);
      }
    }
    return std::make_pair(coords, faces);
  }

  template <class K>
  std::pair<algebra::Vector3Ds,Ints >
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
                           const base::Vector< algebra::Plane3D >&planes) {
  typename CGAL::Nef_polyhedron_3<K> cur(create_cube<K>(bb));
  IMP_INTERNAL_CHECK(cur.is_simple(), "Something wrong with cube ");
  for (unsigned int i=0; i< planes.size(); ++i) {
    cur= cur.intersection(trp<K>(planes[i]),
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

base::Vector<algebra::Vector3Ds >
get_polyhedron_facets(const algebra::BoundingBoxD<3> &bb,
                      const base::Vector< algebra::Plane3D > &outer,
                      const base::Vector< algebra::Plane3D > &hole) {
  CGAL::Nef_polyhedron_3<EKernel> pouter= create_nef<EKernel>(bb, outer);
  CGAL::Nef_polyhedron_3<EKernel> phole= create_nef<EKernel>(bb, hole);
  CGAL::Nef_polyhedron_3<EKernel> diff= pouter-phole;
  return get_facets(diff);
}

std::pair<algebra::Vector3Ds,Ints >
get_polyhedron_indexed_facets(const algebra::BoundingBoxD<3> &bb,
                              const base::Vector< algebra::Plane3D > &outer,
                              const base::Vector< algebra::Plane3D > &hole) {
  CGAL::Nef_polyhedron_3<EKernel> pouter= create_nef<EKernel>(bb, outer);
  CGAL::Nef_polyhedron_3<EKernel> phole= create_nef<EKernel>(bb, hole);
  CGAL::Nef_polyhedron_3<EKernel> diff= pouter-phole;
  return get_indexed_facets(diff);
}


base::Vector<algebra::Vector3Ds >
get_polyhedron_facets(const algebra::BoundingBoxD<3> &bb,
                      const base::Vector< algebra::Plane3D > &outer) {
  CGAL::Nef_polyhedron_3<EKernel> pouter= create_nef<EKernel>(bb, outer);
  return get_facets(pouter);
}



std::pair<algebra::Vector3Ds,Ints >
get_skin_surface(const algebra::Sphere3Ds &ss) {
  IMP_FUNCTION_LOG;
  typedef IKernel::Point_3                                     Bare_point;
  typedef CGAL::Weighted_point<Bare_point,IKernel::RT>         Weighted_point;
  base::Vector<Weighted_point> l(ss.size());
  for (unsigned int i=0; i< ss.size(); ++i) {
    l[i]= Weighted_point(trp<IKernel>(ss[i].get_center()),
                         algebra::get_squared(ss[i].get_radius()));
  }
  CGAL::Polyhedron_3<IKernel> p;
  CGAL::Union_of_balls_3<CGAL::Skin_surface_traits_3<IKernel> >
    skin_surface(l.begin(), l.end());
  CGAL::mesh_skin_surface_3(skin_surface, p);
  //CGAL::make_skin_surface_mesh_3(p, l.begin(), l.end(), 1.0);
  namespace SMS= CGAL::Surface_mesh_simplification;
  /*SMS::Count_stop_predicate<CGAL::Polyhedron_3<IKernel> > stop(10*ss.size());
  IMP_LOG_TERSE( "Simplifying polyhedron from "
          << std::distance(p.facets_begin(), p.facets_end())
          << " faces from " << ss.size() << " balls" << std::endl);*/
  /*SMS::edge_collapse(p, stop,
                  CGAL::vertex_index_map(boost::get(CGAL::vertex_external_index,
                                                       p))
                     .edge_index_map(boost::get(CGAL::edge_external_index, p)));
  IMP_LOG_TERSE( "Simplified polyhedron to "
  << std::distance(p.facets_begin(), p.facets_end()) << std::endl);*/
  return get_indexed_facets(p);
}



namespace {

  typedef CGAL::Surface_mesh_default_triangulation_3 Tr;

  // c2t3
  typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;
  typedef Tr::Geom_traits GT;
  typedef GT::Sphere_3 Sphere_3;
  typedef GT::Point_3 Point_3;
  typedef GT::FT FT;


  template <class Grid>
  class CGALImplicitSurface {
    const Grid &grid_;
    double d_;
  public:
    CGALImplicitSurface(const Grid &grid, double iso_level):
      grid_(grid), d_(iso_level)
    {}
    FT operator()(const Point_3 &pt) const {
      double ret=  get_trilinearly_interpolated(grid_,
                               algebra::Vector3D(pt[0],
                                                 pt[1],
                                                 pt[2]),
                                                typename Grid::Value(d_-1))
        -d_;
      //std::cout << "trilerp at " << trp(pt) << " is " << ret+d_ << std::endl;
      return ret;
    }
    Sphere_3 get_bounding_sphere() const {
      algebra::Vector3D center;
      bool found=false;
      IMP_GRID3D_FOREACH_VOXEL(grid_,{
          typename Grid::ExtendedIndex ei(voxel_index[0],
                                          voxel_index[1],
                                          voxel_index[2]);
          double v= grid_[grid_.get_index(ei)];
          IMP_UNUSED(loop_voxel_index);
          if (v > d_) {
            center= voxel_center;
            found=true;
            goto done;
          }
        });
      if (!found) {
        return Sphere_3(Point_3(0,0,0), 0);
      }
    done:
      algebra::Vector3Ds vt
        = algebra::get_vertices(algebra::get_bounding_box(grid_));
      double max2=0;
      for (unsigned int i=0; i< vt.size(); ++i) {
        double d= algebra::get_squared_distance(vt[i], center);
        if (d>max2) max2=d;
      }
      return Sphere_3(Point_3(center[0], center[1], center[2]), max2);
    }

    double get_scale() const {
      return grid_.get_unit_cell()[0];
    }
  };


  template <class Grid>
  void cgal_triangulate_surface(const Grid &grid,
                                double iso_level, C2t3 &c2t3) {

    // defining the surface
    CGALImplicitSurface<Grid> cs(grid, iso_level);
    Sphere_3 bs= cs.get_bounding_sphere();
    //bs= Sphere_3(Point_3(30, 30, 0), 200);
    IMP_LOG_TERSE( "Bounding sphere is" << bs << std::endl);
    IMP_LOG_TERSE( "Scale is " << cs.get_scale() << std::endl);
    typedef CGAL::Implicit_surface_3<GT, CGALImplicitSurface<Grid> > Surface_3;

    Surface_3 surface(cs,             // pointer to function
                      bs); // bounding sphere

    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<Tr>
      criteria(10.,  // angular bound
               .5*cs.get_scale(),  // radius bound
               .2*cs.get_scale()); // distance bound
    // meshing surface
    //std::cerr << "Computing mesh with seed " << emf.centers()[i] << std::endl;
    IMP_LOG_TERSE( "Beginning surface meshing." << std::endl);
    CGAL::make_surface_mesh(c2t3, surface, criteria,
                            CGAL::Manifold_tag());
    IMP_LOG_TERSE( "Ending surface meshing." << std::endl);
    //CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Manifold_tag());

  }

  template <class Tr, class F>
  const typename Tr::Vertex_handle get_vertex(const Tr &tr, const F &f,
                                              unsigned int i) {
    //return f->first->vertex((f->second+i)%4)->point();
    return f->first->vertex(tr.vertex_triple_index(f->second, i));
  }

  /*template <class HDS, class C2T3>
  class Build_triangle : public CGAL::Modifier_base<HDS> {
    C2T3 &c2t3_;
  public:
    Build_triangle( C2T3 &c2t3): c2t3_(c2t3) {}
    void operator()( HDS& hds) {
      // Postcondition: `hds' is a valid polyhedral surface.
      CGAL::Polyhedron_incremental_builder_3<HDS> B( hds, true);
      B.begin_surface( std::distance(c2t3_.vertices_begin(),
                                     c2t3_.vertices_end()),
                       c2t3_.number_of_facets());
      typedef typename HDS::Vertex   Vertex;
      typedef typename Vertex::Point Point;

      std::map<C2t3::Vertex_handle, int> map;
      for (C2t3::Vertex_iterator it= c2t3_.vertices_begin();
           it != c2t3_.vertices_end(); ++it) {
        int sz= map.size();
        Point pt(it->point()[0],
                 it->point()[1],
                 it->point()[2]);
        map[it]=sz;
        B.add_vertex(pt);
      }
      for (C2t3::Facet_iterator it= c2t3_.facets_begin();
           it != c2t3_.facets_end(); ++it) {
        //B.begin_facet();
        Ints facet;
        for (unsigned int i=0; i< 3; ++i) {
          facet.push_back(map.find(get_vertex(c2t3_.triangulation(),
                                                    it, i))->second);
        }
        if (B.test_facet(facet.begin(), facet.end())) {
          B.add_facet(facet.begin(), facet.end());
        } else {
          B.add_facet(facet.rbegin(), facet.rend());
        }
        //B.end_facet();
      }
      B.end_surface();
    }
    };*/


  template <class Grid>
  std::pair<algebra::Vector3Ds,Ints >
  get_iso_surface_t(const Grid &grid, double iso_level) {
    Tr tr;            // 3D-Delaunay triangulation
    C2t3 c2t3 (tr);   // 2D-complex in 3D-Delaunay triangulation
    cgal_triangulate_surface(grid, iso_level, c2t3);
    /*std::pair<algebra::Vector3Ds,Ints > ret;
    std::map<C2t3::Vertex_handle, int> map;
    for (C2t3::Vertex_iterator it= c2t3.vertices_begin();
         it != c2t3.vertices_end(); ++it) {
      int sz= map.size();
      map[it]=sz;
      ret.first.push_back(trp(it->point()));
    }
    for (C2t3::Facet_iterator it= c2t3.facets_begin();
         it != c2t3.facets_end(); ++it) {
      //B.begin_facet();
      Ints facet;
      for (unsigned int i=0; i< 3; ++i) {
        facet.push_back(map.find(get_vertex(c2t3.triangulation(),
                                            it, i))->second);
      }
      algebra::Vector3D n=
        algebra::get_vector_product(ret.first[facet[1]]- ret.first[facet[0]],
                              ret.first[facet[2]]- ret.first[facet[0]]);
      double gc=algebra::get_trilinearly_interpolated(grid,
                                  ret.first[facet[0]]+n.get_unit_vector());
      if (gc < iso_level) {
        ret.second.insert(ret.second.end(), facet.begin(), facet.end());
      } else {
        ret.second.insert(ret.second.end(), facet.rbegin(), facet.rend());
      }
      ret.second.push_back(-1);
    }*/
    CGAL::Polyhedron_3<GT> poly;
    CGAL::output_surface_facets_to_polyhedron(c2t3, poly);
    /*typedef CGAL::Polyhedron_3<IKernel>         Polyhedron;
    typedef Polyhedron::HalfedgeDS             HalfedgeDS;
    Polyhedron p;
    Build_triangle<HalfedgeDS, C2t3> triangle(c2t3);
    std::cout << "building polyhedron" << std::endl;
    p.delegate( triangle);
    std::cout << "returning facets" << std::endl;*/
    return get_indexed_facets(poly);
  }

}



std::pair<algebra::Vector3Ds,Ints >
get_iso_surface(const algebra::GridD<3,
                                   algebra::DenseGridStorageD<3,
                            double>, double > &grid, double iso_level) {
  IMP_FUNCTION_LOG;
  return get_iso_surface_t(grid, iso_level);
}


std::pair<algebra::Vector3Ds,Ints >
get_iso_surface(const algebra::GridD<3,
                               algebra::DenseGridStorageD<3,
                                    float>, float > &grid, double iso_level) {
  IMP_FUNCTION_LOG;
  return get_iso_surface_t(grid, iso_level);
}

IMPCGAL_END_INTERNAL_NAMESPACE
