%include "RMF/traits.h"
%pythoncode %{
int_tag = IntTag()
float_tag = FloatTag()
string_tag = StringTag()
vector3_tag = Vector3Tag()
vector4_tag = Vector4Tag()
ints_tag = IntsTag()
floats_tag = FloatsTag()
strings_tag = StringsTag()
vector3s_tag = Vector3sTag()
#vector4s_tag = Vector4sTag()
_tag_list = [int_tag, float_tag, string_tag, vector3_tag, vector4_tag,ints_tag, floats_tag, strings_tag, vector3s_tag]
  %}
