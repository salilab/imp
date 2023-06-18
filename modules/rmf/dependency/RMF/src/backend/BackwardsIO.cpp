#include <array>
#include <utility>

#include "BackwardsIO.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace backends {
namespace {
std::array<std::string, 3> make_array(std::string a, std::string b,
                                        std::string c) {
  std::array<std::string, 3> ret;
  ret[0] = a;
  ret[1] = b;
  ret[2] = c;
  return ret;
}
std::array<std::string, 4> make_array(std::string a, std::string b,
                                        std::string c, std::string d) {
  std::array<std::string, 4> ret;
  ret[0] = a;
  ret[1] = b;
  ret[2] = c;
  ret[3] = d;
  return ret;
}
}
typedef std::pair<std::string, std::array<std::string, 3> > P3;
const P3 vector_3_names[] = {
    P3("coordinates", make_array("cartesian x", "cartesian y", "cartesian z")),
    P3("translation",
       make_array("reference frame cartesian x", "reference frame cartesian y",
                  "reference frame cartesian z")),
    P3("rgb color",
       make_array("rgb color red", "rgb color green", "rgb color blue")),
    P3("force", make_array("force cartesian x", "force cartesian y",
                           "force cartesian z")),
    P3("torque", make_array("torque cartesian x", "torque cartesian y",
                            "torque cartesian z"))};
const int vector_3_names_size =
    sizeof(vector_3_names) /
    sizeof(std::pair<std::string, std::array<std::string, 3> >);

V3N vector_3_names_map(vector_3_names, vector_3_names + vector_3_names_size);

typedef std::pair<std::string, std::array<std::string, 4> > P4;

const P4 vector_4_names[] = {
    P4("orientation", make_array("orientation r", "orientation i",
                                 "orientation j", "orientation k")),
    P4("rotation", make_array("reference frame orientation r",
                              "reference frame orientation i",
                              "reference frame orientation j",
                              "reference frame orientation k"))};
const int vector_4_names_size =
    sizeof(vector_4_names) /
    sizeof(std::pair<std::string, std::array<std::string, 4> >);

V4N vector_4_names_map(vector_4_names, vector_4_names + vector_4_names_size);

const P3 vectors_3_names[] = {
    P3("coordinates list",
       make_array("cartesian xs", "cartesian ys", "cartesian zs"))};
const int vectors_3_names_size =
    sizeof(vectors_3_names) /
    sizeof(std::pair<std::string, std::array<std::string, 3> >);

V3N vectors_3_names_map(vectors_3_names,
                        vectors_3_names + vectors_3_names_size);
}
}

RMF_DISABLE_WARNINGS
