#include "ErrorHandeling.h"


EMBD_WrongValue::EMBED_WrongValue(string variable,float val) {
  var=variable;
  value=val;
}

EMBED_IOException::EMBED_IOException(string s) {
  txt=s;
}


virtual const char*  EMBED_IOException::what() const throw() {
  ostringstream msg;
  msg <<  txt ;
  return msg.str().c_str();
}
