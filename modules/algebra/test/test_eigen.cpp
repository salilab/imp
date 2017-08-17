#include <Eigen/Dense>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test Eigen");
  Eigen::ArrayXXf array(5, 4);
  Eigen::MatrixXf matrix(10, 10);
  return 0;
}
