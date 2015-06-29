#include <assert.h>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/distance.hpp>
#include <string>

#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"

int main(int, char * []) {
  std::string fname = "ranges.rmf";
  RMF::FileHandle fh = RMF::create_rmf_file(fname);
  assert(boost::distance(fh.get_node_ids()) == 1);
  fh.add_frame("hi", RMF::FRAME);
  assert(boost::distance(fh.get_frames()) == 1);
  int count = 0;
  RMF_FOREACH(RMF::NodeID n, fh.get_node_ids()) {
    RMF_UNUSED(n);
    ++count;
  }
  assert(count = 1);
  return 0;
}
