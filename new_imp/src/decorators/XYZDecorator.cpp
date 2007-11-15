/**
 *  \file XYZDecorator.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <IMP/decorators/XYZDecorator.h>
#include <sstream>

namespace IMP
{

bool XYZDecorator::keys_initialized_=false;
FloatKey XYZDecorator::x_key_;
FloatKey XYZDecorator::y_key_;
FloatKey XYZDecorator::z_key_;




void XYZDecorator::show(std::ostream &out) const
{
  out << "(" << get_x(), ", "
  << get_y() << ", " << get_z() <<")";

}





void XYZDecorator::initialize_static_data()
{
  if (keys_initialized_) return;
  else {
    x_key_= FloatKey("x");
    y_key_= FloatKey("y");
    z_key_= FloatKey("z");
    keys_initialized_=true;
  }
}

} // namespace IMP
