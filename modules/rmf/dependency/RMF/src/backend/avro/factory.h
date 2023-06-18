
#ifndef SRC_BACKEND_AVRO2_FACTORY__H_
#define SRC_BACKEND_AVRO2_FACTORY__H_

#include <vector>

#include "RMF/config.h"
#include "backend/IOFactory.h"

namespace RMF {
namespace backends {
class IOFactory;
}  // namespace backends
}  // namespace RMF
namespace boost {
template <class Y>
class shared_ptr;
}  // namespace boost

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro2 {
RMFEXPORT std::vector<std::shared_ptr<backends::IOFactory> > get_factories();
}
}

RMF_DISABLE_WARNINGS
#endif
