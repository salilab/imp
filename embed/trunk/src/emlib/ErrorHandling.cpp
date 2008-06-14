#include "ErrorHandling.h"


EMBED_WrongValue::EMBED_WrongValue(std::string variable,float val) {
  var=variable;
  value=val;
}

EMBED_IOException::EMBED_IOException(std::string s) {
  txt=s;
}
