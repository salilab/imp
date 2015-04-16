/** This file is in the public domain.

 */

#include <IMP/log.h>
#include <IMP/flags.h>

int main(int argc, char* argv[]) {
  IMP::setup_from_argv(argc, argv, "An example application");
  std::cout << "Hello world" << std::endl;
  return 0;
}
