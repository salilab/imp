/**
 * \file AnchorPointSubset.cpp
 * \brief Holds a discrete sampling set of
         anchor points and indexes the attributes
 *
 */

#include <IMP/misc/AnchorPointSubset.h>
IMPMISC_BEGIN_NAMESPACE
AnchorPointSubset::AnchorPointSubset() {
  atts_.push_back(FloatKey("x"));
  atts_.push_back(FloatKey("y"));
  atts_.push_back(FloatKey("z"));
}

Float AnchorPointSubset::get_state_val_x(unsigned int i) const {
  return states_[i]->get_value(atts_[0]);
}
Float AnchorPointSubset::get_state_val_y(unsigned int i) const {
  return states_[i]->get_value(atts_[1]);
}
Float AnchorPointSubset::get_state_val_z(unsigned int i) const {
  return states_[i]->get_value(atts_[2]);
}
IMPMISC_END_NAMESPACE
