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
          double eps) {
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
  report(oss.str(), runtime, result/pts.size());
}

template <class KNN>
void test_uniform(std::string name,
                  unsigned int n, double eps) {
  set_log_level(SILENT);
  set_check_level(IMP::NONE);
  BoundingBox3D bb(VectorD<3>(0,0,0),
                   VectorD<3>(10,10,10));
  std::vector<VectorD<3> > pts;
  for (unsigned int i=0; i< n; ++i) {
    pts.push_back(get_random_vector_in(bb));
  }
  KNN knn(pts.begin(), pts.end());
  test(name+" uniform", pts, knn, eps);
}

int main() {
#ifdef IMP_USE_ANN
  {
    std::string name("ann");
    typedef IMP::algebra::internal::ANNData<3> Data;
    test_uniform<Data>(name, 10, 0);
    test_uniform<Data>(name, 10, .1);
    test_uniform<Data>(name, 10, .5);
    test_uniform<Data>(name, 100, 0);
    test_uniform<Data>(name, 100, .1);
    test_uniform<Data>(name, 100, .5);
    test_uniform<Data>(name, 1000, 0);
    test_uniform<Data>(name, 1000, .1);
    test_uniform<Data>(name, 1000, .5);
}
#endif
#ifdef IMP_USE_CGAL
  {
    std::string name("cgal");
    typedef IMP::algebra::internal::CGALKNNData<3> Data;
    test_uniform<Data>(name, 10, 0);
    test_uniform<Data>(name, 10, .1);
    test_uniform<Data>(name, 10, .5);
    test_uniform<Data>(name, 100, 0);
    test_uniform<Data>(name, 100, .1);
    test_uniform<Data>(name, 100, .5);
    test_uniform<Data>(name, 1000, 0);
    test_uniform<Data>(name, 1000, .1);
    test_uniform<Data>(name, 1000, .5);
}
#endif
  {
    std::string name("linear");
    typedef IMP::algebra::internal::LinearKNNData<3> Data;
    test_uniform<Data>(name, 10, 0);
    test_uniform<Data>(name, 10, .1);
    test_uniform<Data>(name, 10, .5);
    test_uniform<Data>(name, 100, 0);
    test_uniform<Data>(name, 100, .1);
    test_uniform<Data>(name, 100, .5);
    test_uniform<Data>(name, 1000, 0);
    test_uniform<Data>(name, 1000, .1);
    test_uniform<Data>(name, 1000, .5);
}
  return 0;
}
