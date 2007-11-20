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

FloatKey XYZDecorator::x_key_;
FloatKey XYZDecorator::y_key_;
FloatKey XYZDecorator::z_key_;




  void XYZDecorator::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" << get_x()<< ", "
  << get_y() << ", " << get_z() <<")";

}




  IMP_DECORATOR_INITIALIZE(XYZDecorator, DecoratorBase,
                           {
                             x_key_= FloatKey("x");
                             y_key_= FloatKey("y");
                             z_key_= FloatKey("z");
                           })

} // namespace IMP
