#include "IMP.h"

namespace imp
{

Model* the_model = NULL;

Model& get_model(void)
{
  if (the_model == NULL) {
    the_model = new Model();
  }

  return *the_model;
}

} // namespace imp
