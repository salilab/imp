/** \example base/flags.cpp
    Show simple usage of the command line flags support in IMP.base in a
    C++ executable. Run with "--help" to see all available flags.
*/
#include <IMP/base/flags.h>

int main(int argc, char *argv[]) {
  std::string hello = "Hello world";
  IMP::base::AddStringFlag("hello", "How to say hello", &hello);

  IMP::base::setup_from_argv(argc, argv, "Example on C++ use of flags.");

  if (hello.empty()) {
    // bad argument
    IMP::base::write_help();
    return 1;
  }

  std::cout << hello << std::endl;

  return 0;
}
