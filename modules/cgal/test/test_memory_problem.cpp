#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_3.h>
#include <CGAL/Alpha_shape_vertex_base_3.h>
#include <CGAL/Triangulation_cell_base_3.h>
#include <CGAL/Alpha_shape_cell_base_3.h>
//#include <CGAL/Triangulation_data_structure_3.h>
#include <CGAL/Weighted_alpha_shape_euclidean_traits_3.h>
#include <CGAL/Regular_triangulation_3.h>
#include <CGAL/version.h>

#include <CGAL/Fixed_alpha_shape_3.h>
#include <CGAL/Fixed_alpha_shape_vertex_base_3.h>
#include <CGAL/Fixed_alpha_shape_cell_base_3.h>

#include <CGAL/Alpha_shape_3.h>


typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef K::Point_3 Point;
typedef K::Segment_3 Segment;
typedef K::Vector_3 Vector;
typedef K::Line_3 Line;
typedef K::Triangle_3 Triangle3;

typedef CGAL::Regular_triangulation_euclidean_traits_3<K> Gt;
typedef CGAL::Fixed_alpha_shape_vertex_base_3<Gt> Vb;
typedef CGAL::Fixed_alpha_shape_cell_base_3<Gt> Fb;
typedef CGAL::Triangulation_data_structure_3<Vb, Fb> TDS;
typedef CGAL::Regular_triangulation_3<Gt, TDS> Triangulation;
typedef Gt::Point Wpoint;


int main(int, char * []) {
  std::cout << CGAL_VERSION_NR << std::endl;
  std::vector<Wpoint> pts;
  for (unsigned int i = 0; i < 1200; ++i) {
    pts.push_back(Wpoint(Point(i,i,i),
                         1));
  }

  Triangulation T(pts.begin(), pts.end());

  return 0;
}
