/**
 *  \file internal/cgal_knn.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#include <IMP/cgal/internal/polyhedrons.h>
#include <IMP/log.h>
#include <IMP/log_macros.h>
#include <IMP/algebra/grid_utility.h>
IMP_GCC_PRAGMA(diagnostic ignored "-Wuninitialized")
#include <CGAL/Origin.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Handle_hash_function.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/make_skin_surface_mesh_3.h>
#include <CGAL/Union_of_balls_3.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/version.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/make_mesh_3.h>
#if CGAL_VERSION_NR >= 1041300000
#include <CGAL/Labeled_mesh_domain_3.h>
#else
#include <CGAL/Implicit_mesh_domain_3.h>
#endif
#if CGAL_VERSION_NR < 1050101000
#include <CGAL/Surface_mesh_simplification/HalfedgeGraph_Polyhedron_3.h>
#endif

namespace {
typedef CGAL::Exact_predicates_exact_constructions_kernel EKernel;
typedef CGAL::Exact_predicates_inexact_constructions_kernel IKernel;
}
namespace CGAL {
namespace internal {
unsigned int hash_value(CGAL::Polyhedron_3<EKernel>::Vertex_handle vh);
unsigned int hash_value(CGAL::Polyhedron_3<EKernel>::Vertex_handle vh) {
  return CGAL::Handle_hash_function()(vh);
}
bool operator<(CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
               CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1);
bool operator<(CGAL::Polyhedron_3<EKernel>::Vertex_handle vh0,
               CGAL::Polyhedron_3<EKernel>::Vertex_handle vh1) {
  return &*vh0 < &*vh1;
}
}
namespace internal {
unsigned int hash_value(CGAL::Polyhedron_3<IKernel>::Vertex_handle vh);
bool operator<(CGAL::Polyhedron_3<IKernel>::Vertex_handle vh0,
               CGAL::Polyhedron_3<IKernel>::Vertex_handle vh1);
unsigned int hash_value(CGAL::Polyhedron_3<IKernel>::Vertex_handle vh) {
  return CGAL::Handle_hash_function()(vh);
}
bool operator<(CGAL::Polyhedron_3<IKernel>::Vertex_handle vh0,
               CGAL::Polyhedron_3<IKernel>::Vertex_handle vh1) {
  return &*vh0 < &*vh1;
}
}
}

IMPCGAL_BEGIN_INTERNAL_NAMESPACE
namespace {

struct AddressLess {
  template <class T>
  bool operator()(const T &a, const T &b) const {
    return &*a < &*b;
  }
};

template <class K>
algebra::Vector3D trp(const typename CGAL::Point_3<K> pt) {
  return algebra::Vector3D(CGAL::to_double(pt[0]), CGAL::to_double(pt[1]),
                           CGAL::to_double(pt[2]));
}
template <class K>
typename K::Point_3 trp(const algebra::Vector3D pt) {
  return typename K::Point_3(pt[0], pt[1], pt[2]);
}
template <class K>
typename K::Plane_3 trp(const algebra::Plane3D &p) {
  return typename K::Plane_3(p.get_normal()[0], p.get_normal()[1],
                             p.get_normal()[2], -p.get_distance_from_origin());
}

template <class K>
IMP::Vector<algebra::Vector3Ds> get_facets(CGAL::Nef_polyhedron_3<K> &np) {
  typename CGAL::Polyhedron_3<K> p;
  np.convert_to_polyhedron(p);
  CGAL_postcondition(p.is_valid());
  IMP::Vector<IMP::Vector<typename algebra::Vector3D>> ret;
  for (typename CGAL::Polyhedron_3<K>::Face_iterator it = p.facets_begin();
       it != p.facets_end(); ++it) {
    ret.push_back(IMP::Vector<algebra::Vector3D>());
    typename CGAL::Polyhedron_3<K>::Facet::Halfedge_around_facet_circulator c =
        it->facet_begin();
    do {
      ret.back().push_back(trp<K>(c->vertex()->point()));
      ++c;
    } while (c != it->facet_begin());
  }
  return ret;
}

template <class K>
std::pair<algebra::Vector3Ds, Ints> get_indexed_facets(
    CGAL::Polyhedron_3<K> &p) {
  Ints faces;
  algebra::Vector3Ds coords;
  typename std::map<typename CGAL::Polyhedron_3<K>::Vertex_handle, int,
                    AddressLess> vertices;
  for (typename CGAL::Polyhedron_3<K>::Face_iterator it = p.facets_begin();
       it != p.facets_end(); ++it) {
    typename CGAL::Polyhedron_3<K>::Facet::Halfedge_around_facet_circulator c =
        it->facet_begin();
    do {
      typename CGAL::Polyhedron_3<K>::Vertex_handle vh = c->vertex();
      if (vertices.find(vh) == vertices.end()) {
        vertices[vh] = coords.size();
        coords.push_back(trp(vh->point()));
      }
      faces.push_back(vertices.find(vh)->second);
      ++c;
    } while (c != it->facet_begin());
    if (!faces.empty() && faces.back() != -1) {
      faces.push_back(-1);
    }
  }
  return std::make_pair(coords, faces);
}

template <class K>
std::pair<algebra::Vector3Ds, Ints> get_indexed_facets(
    typename CGAL::Nef_polyhedron_3<K> &np) {
  typename CGAL::Polyhedron_3<K> p;
  np.convert_to_polyhedron(p);
  return get_indexed_facets(p);
}

template <class K>
#if CGAL_VERSION_NR >= 1050421000
CGAL::Polyhedron_3<K> create_cube(const algebra::BoundingBoxD<3> &bb) {
#else
CGAL::Nef_polyhedron_3<K> create_cube(const algebra::BoundingBoxD<3> &bb) {
#endif
  typename CGAL::Polyhedron_3<K> p;
  // appends a cube of size [0,1]^3 to the polyhedron P.
  CGAL_precondition(p.is_valid());
  typedef typename CGAL::Polyhedron_3<K>::Halfedge_handle Halfedge_handle;
  Halfedge_handle h = p.make_tetrahedron(
      typename K::Point_3(bb.get_corner_1()[0], bb.get_corner_0()[1],
                          bb.get_corner_0()[2]),
      typename K::Point_3(bb.get_corner_0()[0], bb.get_corner_0()[1],
                          bb.get_corner_1()[2]),
      typename K::Point_3(bb.get_corner_0()[0], bb.get_corner_0()[1],
                          bb.get_corner_0()[2]),
      typename K::Point_3(bb.get_corner_0()[0], bb.get_corner_1()[1],
                          bb.get_corner_0()[2]));
  Halfedge_handle g = h->next()->opposite()->next();  // Fig. (a)
  p.split_edge(h->next());
  p.split_edge(g->next());
  p.split_edge(g);  // Fig. (b)
  h->next()->vertex()->point() = typename K::Point_3(
      bb.get_corner_1()[0], bb.get_corner_0()[1], bb.get_corner_1()[2]);
  g->next()->vertex()->point() = typename K::Point_3(
      bb.get_corner_0()[0], bb.get_corner_1()[1], bb.get_corner_1()[2]);
  g->opposite()->vertex()->point() = typename K::Point_3(
      bb.get_corner_1()[0], bb.get_corner_1()[1], bb.get_corner_0()[2]);
  // Fig. (c)
  Halfedge_handle f =
      p.split_facet(g->next(), g->next()->next()->next());  // Fig. (d)
  Halfedge_handle e = p.split_edge(f);
  e->vertex()->point() =
      typename K::Point_3(bb.get_corner_1()[0], bb.get_corner_1()[1],
                          bb.get_corner_1()[2]);  // Fig. (e)
  p.split_facet(e, f->next()->next());           // Fig. (f)
  CGAL_postcondition(p.is_valid());
  return p;
}

template <class K>
CGAL::Nef_polyhedron_3<K> create_nef(
    const algebra::BoundingBoxD<3> &bb,
    const IMP::Vector<algebra::Plane3D> &planes) {
  typename CGAL::Nef_polyhedron_3<K> cur(create_cube<K>(bb));
  IMP_INTERNAL_CHECK(cur.is_simple(), "Something wrong with cube ");
  for (unsigned int i = 0; i < planes.size(); ++i) {
    cur = cur.intersection(trp<K>(planes[i]),
                           CGAL::Nef_polyhedron_3<K>::CLOSED_HALFSPACE);
    typename CGAL::Polyhedron_3<K> p;
    try {
      cur.convert_to_polyhedron(p);
      // std::cout << p << std::endl;
    }
    catch (...) {
      IMP_INTERNAL_CHECK(false, "Error after intersection with "
                                    << i << " which is " << planes[i]);
    }
  }
  IMP_INTERNAL_CHECK(cur.is_simple(), "Something wrong with cut cube ");
  return cur;
}
}

IMP::Vector<algebra::Vector3Ds> get_polyhedron_facets(
    const algebra::BoundingBoxD<3> &bb,
    const IMP::Vector<algebra::Plane3D> &outer,
    const IMP::Vector<algebra::Plane3D> &hole) {
  CGAL::Nef_polyhedron_3<EKernel> pouter = create_nef<EKernel>(bb, outer);
  CGAL::Nef_polyhedron_3<EKernel> phole = create_nef<EKernel>(bb, hole);
  CGAL::Nef_polyhedron_3<EKernel> diff = pouter - phole;
  return get_facets(diff);
}

std::pair<algebra::Vector3Ds, Ints> get_polyhedron_indexed_facets(
    const algebra::BoundingBoxD<3> &bb,
    const IMP::Vector<algebra::Plane3D> &outer,
    const IMP::Vector<algebra::Plane3D> &hole) {
  CGAL::Nef_polyhedron_3<EKernel> pouter = create_nef<EKernel>(bb, outer);
  CGAL::Nef_polyhedron_3<EKernel> phole = create_nef<EKernel>(bb, hole);
  CGAL::Nef_polyhedron_3<EKernel> diff = pouter - phole;
  return get_indexed_facets(diff);
}

IMP::Vector<algebra::Vector3Ds> get_polyhedron_facets(
    const algebra::BoundingBoxD<3> &bb,
    const IMP::Vector<algebra::Plane3D> &outer) {
  CGAL::Nef_polyhedron_3<EKernel> pouter = create_nef<EKernel>(bb, outer);
  return get_facets(pouter);
}

std::pair<algebra::Vector3Ds, Ints> get_skin_surface(
    const algebra::Sphere3Ds &ss) {
  IMP_FUNCTION_LOG;
#if CGAL_VERSION_NR > 1040911000
  typedef IKernel::Weighted_point_3 Weighted_point;
#else
  typedef IKernel::Point_3 Bare_point;
  typedef CGAL::Weighted_point<Bare_point, IKernel::RT> Weighted_point;
#endif
  IMP::Vector<Weighted_point> l(ss.size());
  for (unsigned int i = 0; i < ss.size(); ++i) {
    l[i] = Weighted_point(trp<IKernel>(ss[i].get_center()),
                          algebra::get_squared(ss[i].get_radius()));
  }
  CGAL::Polyhedron_3<IKernel> p;
  CGAL::Union_of_balls_3<CGAL::Skin_surface_traits_3<IKernel>> skin_surface(
      l.begin(), l.end());
  CGAL::mesh_skin_surface_3(skin_surface, p);
  return get_indexed_facets(p);
}

namespace {

typedef IKernel::Point_3 Point_3;
typedef IKernel::Sphere_3 Sphere_3;
typedef IKernel::FT FT;

/* Implicit function for the 3D Mesh Generation package; the domain to be
   meshed is the region where the function is negative, i.e. where the
   grid value is greater than the iso level */
template <class Grid>
class CGALImplicitSurface {
  const Grid &grid_;
  double d_;

  algebra::Vector3D get_center(bool &found) const {
    algebra::Vector3D center;
    found = false;
    IMP_GRID3D_FOREACH_VOXEL(grid_, {
      typename Grid::ExtendedIndex ei(voxel_index[0], voxel_index[1],
                                      voxel_index[2]);
      double v = grid_[grid_.get_index(ei)];
      IMP_UNUSED(loop_voxel_index);
      if (v > d_) {
        center = voxel_center;
        found = true;
        return center;
      }
    });
    return center;
  }

 public:
  CGALImplicitSurface(const Grid &grid, double iso_level)
      : grid_(grid), d_(iso_level) {}
  FT operator()(const Point_3 &pt) const {
    return d_ - get_trilinearly_interpolated(
                    grid_, algebra::Vector3D(pt[0], pt[1], pt[2]),
                    typename Grid::Value(d_ - 1));
  }
  // Get a sphere bounding the surface; return false if there is no surface
  bool get_bounding_sphere(Sphere_3 &bs) const {
    bool found;
    algebra::Vector3D center = get_center(found);
    if (!found) {
      return false;
    }
    algebra::Vector3Ds vt =
        algebra::get_vertices(algebra::get_bounding_box(grid_));
    double max2 = 0;
    for (unsigned int i = 0; i < vt.size(); ++i) {
      double d = algebra::get_squared_distance(vt[i], center);
      if (d > max2) max2 = d;
    }
    bs = Sphere_3(Point_3(center[0], center[1], center[2]), max2);
    return true;
  }

  double get_scale() const { return grid_.get_unit_cell()[0]; }
};

template <class Grid>
std::pair<algebra::Vector3Ds, Ints> get_iso_surface_t(const Grid &grid,
                                                      double iso_level) {
  typedef CGALImplicitSurface<Grid> Function;
#if CGAL_VERSION_NR >= 1041300000
  typedef CGAL::Labeled_mesh_domain_3<IKernel> Mesh_domain;
#else
  typedef CGAL::Implicit_mesh_domain_3<Function, IKernel> Mesh_domain;
#endif
  typedef typename CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
  typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
  typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;
  typedef typename Mesh_criteria::Facet_criteria Facet_criteria;
  typedef typename Mesh_criteria::Cell_criteria Cell_criteria;

  Function func(grid, iso_level);
  Sphere_3 bs;
  if (!func.get_bounding_sphere(bs)) {
    return std::make_pair(algebra::Vector3Ds(), Ints());
  }
  IMP_LOG_TERSE("Bounding sphere is" << bs << std::endl);
  IMP_LOG_TERSE("Scale is " << func.get_scale() << std::endl);
#if CGAL_VERSION_NR >= 1041300000
  Mesh_domain domain = Mesh_domain::create_implicit_mesh_domain(func, bs);
#else
  Mesh_domain domain(func, bs);
#endif

  // Only the surface is of interest, so put no constraints on the cells
  Mesh_criteria criteria(Facet_criteria(10.,                    // angle bound
                                        .5 * func.get_scale(),  // radius bound
                                        .2 * func.get_scale()), // distance
                         Cell_criteria(0., 0.));
  IMP_LOG_TERSE("Beginning surface meshing." << std::endl);
  C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria,
                                      CGAL::parameters::no_perturb(),
                                      CGAL::parameters::no_exude()
#if CGAL_VERSION_NR >= 1041200000
                                      , CGAL::parameters::manifold()
#endif
                                      );
  IMP_LOG_TERSE("Ending surface meshing." << std::endl);

  // Extract the boundary facets, oriented so that normals point outwards
  algebra::Vector3Ds coords;
  Ints faces;
  std::map<typename Tr::Vertex_handle, int, AddressLess> vertices;
  for (typename C3t3::Facets_in_complex_iterator it =
           c3t3.facets_in_complex_begin();
       it != c3t3.facets_in_complex_end(); ++it) {
    typename Tr::Cell_handle c = it->first;
    int s = it->second;
    int ind[3];
    for (unsigned int i = 0; i < 3; ++i) {
      typename Tr::Vertex_handle vh =
          c->vertex(Tr::vertex_triple_index(s, i));
      typename std::map<typename Tr::Vertex_handle, int,
                        AddressLess>::const_iterator vit = vertices.find(vh);
      if (vit == vertices.end()) {
        ind[i] = coords.size();
        vertices[vh] = ind[i];
        coords.push_back(trp(vh->point().point()));
      } else {
        ind[i] = vit->second;
      }
    }
    // Facet vertices are oriented with normal pointing into cell c
    if (c3t3.is_in_complex(c)) {
      std::swap(ind[0], ind[1]);
    }
    faces.insert(faces.end(), ind, ind + 3);
    faces.push_back(-1);
  }
  return std::make_pair(coords, faces);
}
}

std::pair<algebra::Vector3Ds, Ints> get_iso_surface(
    const algebra::GridD<3, algebra::DenseGridStorageD<3, double>, double> &
        grid,
    double iso_level) {
  IMP_FUNCTION_LOG;
  return get_iso_surface_t(grid, iso_level);
}

std::pair<algebra::Vector3Ds, Ints> get_iso_surface(
    const algebra::GridD<3, algebra::DenseGridStorageD<3, float>, float> &grid,
    double iso_level) {
  IMP_FUNCTION_LOG;
  return get_iso_surface_t(grid, iso_level);
}

IMPCGAL_END_INTERNAL_NAMESPACE
