/* Why would we copyright a test?
 */
#include <boost/unordered_map.hpp>
#include <IMP/Vector.h>
#include <IMP/flags.h>
IMP_COMPILER_DISABLE_WARNINGS
IMP_CLANG_PRAGMA(diagnostic ignored "-Wc++11-extensions")
#include <CGAL/Origin.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Handle_hash_function.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/make_skin_surface_mesh_3.h>
#include <CGAL/Union_of_balls_3.h>
#include <CGAL/Gmpq.h>
#include <CGAL/version.h>
IMP_COMPILER_ENABLE_WARNINGS

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test skin failure.");
  double pts[][3] = {{3.3874, 3.3577, 2.86547},
                     {4.20832, 3.04325, 3.05838},
                     {3.63033, 2.62921, 2.50657},
                     {4.3492, 2.80494, 1.99437},
                     {5.24092, 2.81322, 2.11588},
                     {6.00076, 3.29489, 2.1409},
                     {5.53583, 3.6421, 1.45294},
                     {5.97231, 2.95352, 1.07171},
                     {5.29922, 3.54395, 0.980338},
                     {5.46575, 3.92853, 0.183865}};
  typedef CGAL::Exact_predicates_inexact_constructions_kernel IKernel;
  typedef IKernel::Point_3 Bare_point;
#if CGAL_VERSION_NR > 1040911000
  typedef IKernel::Weighted_point_3 Weighted_point;
#else
  typedef CGAL::Weighted_point<Bare_point, IKernel::RT> Weighted_point;
#endif
  unsigned int size = sizeof(pts) / (3 * sizeof(double));
  IMP::Vector<Weighted_point> l;
  for (unsigned int i = 0; i < size; ++i) {
    l.push_back(
        Weighted_point(Bare_point(pts[i][0], pts[i][1], pts[i][2]), .9 * .9));
    std::cout << ".color " << i << std::endl;
    std::cout << ".sphere " << pts[i][0] << " " << pts[i][1] << " " << pts[i][2]
              << " " << .9 << std::endl;
  }
  CGAL::Polyhedron_3<IKernel> p;
  CGAL::Union_of_balls_3<CGAL::Skin_surface_traits_3<IKernel> > skin_surface(
      l.begin(), l.end());
  // CGAL::mesh_skin_surface_3(skin_surface, p);
  return 0;
}
