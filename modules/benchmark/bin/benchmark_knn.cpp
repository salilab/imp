/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>

using namespace IMP;
using namespace IMP::algebra;
using namespace IMP::benchmark;

template <class KNN>
void test(std::string name,
          const std::vector<VectorD<3> > &pts,
          const KNN &knn,
          double eps,
          double target) {
  double result=0;
  double runtime;
  IMP_TIME({
      for (unsigned int i=0; i< pts.size(); ++i) {
        Ints ret(2);
        knn.fill_nearest_neighbors(pts[i], 2U, eps, ret);
        result+=ret[1];
      }
    }, runtime);
  std::ostringstream oss;
  oss << "knn " << name << " " << knn.get_number_of_points() << " " << eps;
  report(oss.str(), runtime, target, result/pts.size());
}

template <class KNN>
void test_uniform(std::string name,
                  unsigned int n, double eps, double target) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  BoundingBox3D bb(VectorD<3>(0,0,0),
                   VectorD<3>(10,10,10));
  std::vector<VectorD<3> > pts;
  for (unsigned int i=0; i< n; ++i) {
    pts.push_back(get_random_vector_in(bb));
  }
  KNN knn(pts.begin(), pts.end());
  test(name+" uniform", pts, knn, eps, target);
}

int main() {
#ifdef IMP_USE_ANN
  {
    std::string name("ann");
    typedef IMP::algebra::internal::ANNData<3> Data;
    test_uniform<Data>(name, 10, 0, 0.000424);
    test_uniform<Data>(name, 10, .1, 0.000388);
    test_uniform<Data>(name, 10, .5, 0.000332);
    test_uniform<Data>(name, 100, 0, 0.006414);
    test_uniform<Data>(name, 100, .1, 0.006614);
    test_uniform<Data>(name, 100, .5, 0.005015);
    test_uniform<Data>(name, 1000, 0, 0.096487);
    test_uniform<Data>(name, 1000, .1, 0.088377);
    test_uniform<Data>(name, 1000, .5, 0.069829);
}
#endif
#ifdef IMP_USE_CGAL
  {
    std::string name("cgal");
    typedef IMP::cgal::internal::KNNData<3> Data;
    test_uniform<Data>(name, 10, 0, 0.000673);
    test_uniform<Data>(name, 10, .1, 0.000817);
    test_uniform<Data>(name, 10, .5, 0.000691);
    test_uniform<Data>(name, 100, 0, 0.028238);
    test_uniform<Data>(name, 100, .1, 0.022699);
    test_uniform<Data>(name, 100, .5, 0.020200);
    test_uniform<Data>(name, 1000, 0, 0.412556);
    test_uniform<Data>(name, 1000, .1, 0.378601);
    test_uniform<Data>(name, 1000, .5, 0.323374);
}
#endif
  {
    std::string name("linear");
    typedef IMP::algebra::internal::LinearKNNData<3> Data;
    test_uniform<Data>(name, 10, 0, 0.000329);
    test_uniform<Data>(name, 10, .1, 0.000325);
    test_uniform<Data>(name, 10, .5, 0.000339);
    test_uniform<Data>(name, 100, 0, 0.008030);
    test_uniform<Data>(name, 100, .1, 0.008195);
    test_uniform<Data>(name, 100, .5, 0.008049);
    test_uniform<Data>(name, 1000, 0, 0.464646);
    test_uniform<Data>(name, 1000, .1, 0.462312);
    test_uniform<Data>(name, 1000, .5, 0.465823);
}
  return IMP::benchmark::get_return_value();
}
