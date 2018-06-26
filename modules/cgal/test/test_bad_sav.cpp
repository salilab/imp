/* Why would we copyright a test?
 */

#include <IMP/cgal/internal/union_of_balls.h>
#include <IMP/flags.h>

typedef IMP::algebra::Sphere3D S;
typedef IMP::algebra::Vector3D V;

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test bad sav");
  //
  // test computation for two non intersecting balls
  //
  IMP::algebra::Sphere3Ds balls;
  balls.push_back(S(V(2.441, 12.404, 7.789), 1.85));
  balls.push_back(S(V(396, 13.826, 7.425), 2.275));
  std::pair<double, double> dp =
      IMP::cgal::internal::get_surface_area_and_volume(balls);
  std::cout << "union of balls volumetrics : " << dp.first << " and "
            << dp.second << std::endl;
  double s = IMP::algebra::get_surface_area(balls[0]) +
             IMP::algebra::get_surface_area(balls[1]);
  double v =
      IMP::algebra::get_volume(balls[0]) + IMP::algebra::get_volume(balls[1]);
  std::cout << "sum of volumetrics for the two balls : " << s << " and " << v
            << std::endl;
  double epsilon = 1e-8;
  IMP_UNUSED(epsilon);
  IMP_USAGE_CHECK(dp.first > 0, "Surface area must be positive");
  IMP_USAGE_CHECK(dp.second > 0, "Volume must be positive");
  IMP_INTERNAL_CHECK(abs(dp.first - s) < epsilon,
                     "surface computation mismatch");
  IMP_INTERNAL_CHECK(abs(dp.second - v) < epsilon,
                     "volume computation mismatch");
  return 0;
}
