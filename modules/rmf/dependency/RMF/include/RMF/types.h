/**
 *  \file RMF/types.h
 *  \brief Default implementation for types.h

 *  Use RMF_TYPES_HEADER to replace it with another header.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_TYPES_H
#define RMF_TYPES_H

#include "RMF/config.h"
#include "Vector.h"

namespace RMF {

//! Work around clang issue
typedef unsigned int VectorDimension;

//! For classes that are templated on the dimension
#define RMF_VECTOR RMF::Vector

//! For template classes that want to deal with lists of values
#define RMF_TYPES std::vector

/** The type used to store integral values.*/
typedef int Int;
/** The type used to store lists of integral values.*/
typedef RMF_TYPES<Int> Ints;
/** The type used to store lists of floating point values.*/
typedef float Float;
/** The type used to store lists of floating point values.*/
typedef RMF_TYPES<Float> Floats;
/** The type used to store lists of floating point values.*/
typedef RMF_TYPES<Floats> FloatsList;
/** The type used to store lists of string values.*/
typedef std::string String;
/** The type used to store lists of string values.*/
typedef RMF_TYPES<String> Strings;
/** The type used to store lists of strings values.*/
typedef RMF_TYPES<Strings> StringsList;
/** The type used to store lists of lists of integers values.*/
typedef RMF_TYPES<Ints> IntsList;

/** A Vector3 */
typedef Vector<3U> Vector3;
/** A Vector3 */
typedef Vector<4U> Vector4;
/** Many Vector3s */
typedef RMF_TYPES<Vector3> Vector3s;
/** Many Vector4s */
typedef RMF_TYPES<Vector4> Vector4s;

typedef boost::array<int, 2> IntRange;

} /* namespace RMF */

#endif /* RMF_TYPES_H */
