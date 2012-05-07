/** This file is in the public domain.

 */

#include <IMP/log.h>

int main(int, char*[]) {
  IMP_LOG(IMP::SILENT, "Hello world" << std::endl);
  return 0;
}
