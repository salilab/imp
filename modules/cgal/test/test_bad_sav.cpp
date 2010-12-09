/* Why would we copyright a test?
 */

#include <IMP/cgal/internal/union_of_balls.h>
typedef IMP::algebra::Sphere3D S;
typedef IMP::algebra::Vector3D V;
int main(int, char *[]) {
  std::vector<S> balls;
  balls.push_back(S(V(2.441, 12.404, 7.789), 1.85));
  balls.push_back(S(V(396, 13.826, 7.425), 2.275));
  std::pair<double, double> dp
    = IMP::cgal::internal::get_surface_area_and_volume(balls);
  std::cout << dp.first << " and " << dp.second << std::endl;
  std::cout << IMP::algebra::get_surface_area(balls[0])
                +IMP::algebra::get_surface_area(balls[1])
            << " and "
            << IMP::algebra::get_volume(balls[0])
                +IMP::algebra::get_volume(balls[1])
            << std::endl;
  IMP_USAGE_CHECK(dp.first >0, "Surface area must be positive");
  IMP_USAGE_CHECK(dp.second >0, "Volume must be positive");
  return 0;
}
