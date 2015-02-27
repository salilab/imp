/** This file is in the public domain.

 */

#include <IMP/base/log.h>
#include <IMP/base/flags.h>

int main(int argc, char* argv[]) {
  IMP::base::setup_from_argv(argc, argv, "An example application");
  std::cout << "Hello world" << std::endl;
  return 0;
}
