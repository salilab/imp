/**
 *  \file base_types.cpp   \brief Base types.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/base_types.h"

namespace IMP
{

namespace internal {
unsigned int next_attribute_table_index_ = 0;
std::vector<KeyData> attribute_key_data;
double key_initialization_heuristic=0;
}

IMP_DEFINE_KEY_TYPE(IntKey, Int);
IMP_DEFINE_KEY_TYPE(FloatKey, Float);
IMP_DEFINE_KEY_TYPE(StringKey, String);

IMP_DEFINE_KEY_TYPE(AtomType, AtomTypeTag);
IMP_DEFINE_KEY_TYPE(ResidueType, ResidueTypeTag);

}
