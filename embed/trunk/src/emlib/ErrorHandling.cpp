#include "ErrorHandling.h"


EMBED_WrongValue::EMBED_WrongValue(std::string variable, float val)
{
  var_ = variable;
  value_ = val;
}

EMBED_IOException::EMBED_IOException(std::string s)
{
  txt_ = s;
}
