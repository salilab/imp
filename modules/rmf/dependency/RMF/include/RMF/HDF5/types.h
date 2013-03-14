/**
 *  \file RMF/types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_TYPES_H
#define RMF_HDF5_TYPES_H

#include <RMF/config.h>
#include "handle.h"
#include "infrastructure_macros.h"
#include <RMF/internal/errors.h>
#include "internal/types.h"
#include <hdf5.h>
#include <algorithm>
#include <boost/cstdint.hpp>
#include <limits>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

/** The type used to store integral values.*/
typedef int Int;
/** The type used to store lists of integral values.*/
typedef std::vector<Int> Ints;
/** The type used to store lists of floating point values.*/
typedef double Float;
/** The type used to store lists of floating point values.*/
typedef std::vector<Float> Floats;
/** The type used to store lists of floating point values.*/
typedef std::vector<Floats> FloatsList;
/** The type used to store lists of index values.*/
typedef int Index;
/** The type used to store lists of index values.*/
typedef std::vector<Index> Indexes;
/** The type used to store lists of indexes.*/
typedef std::vector<Indexes> IndexesList;
/** The type used to store lists of string values.*/
typedef std::string String;
/** The type used to store lists of string values.*/
typedef std::vector<String> Strings;
/** The type used to store lists of strings values.*/
typedef std::vector<Strings> StringsList;
/** The type used to store lists of lists of integers values.*/
typedef std::vector<Ints> IntsList;
/** The type used to store char values.*/
typedef char Char;
/** The type used to store lists of char values.*/
typedef std::string Chars;

RMF_SIMPLE_TRAITS(Int,   Ints,   int,   0, H5T_STD_I64LE, H5T_NATIVE_INT,
                  H5T_NATIVE_INT, boost::int32_t,
                  std::numeric_limits<int>::max());

RMF_SIMPLE_TRAITS(Float, Floats, float, 1, H5T_IEEE_F64LE,
                  H5T_NATIVE_DOUBLE,
                  H5T_NATIVE_DOUBLE, double, std::numeric_limits<double>::max());

RMF_SIMPLE_TRAITS(Index, Indexes, index, 2, H5T_STD_I64LE, H5T_NATIVE_INT,
                  H5T_NATIVE_INT, boost::int32_t, -1);

RMF_TRAITS(String, Strings, string, 3, internal::get_string_type(),
           internal::get_string_type(), internal::get_string_type(),
           std::string,
           String(), i.empty(), {
             char *c;
             if (!v.empty()) {
               c = const_cast<char*>(v.c_str());
             } else {
               static char empty = '\0';
               c = &empty;
             }
             RMF_HDF5_CALL(H5Dwrite(d, get_hdf5_memory_type(), is, s,
                                    H5P_DEFAULT, &c));
           }
           , {
             char *c = NULL;
             RMF_HDF5_HANDLE( mt, internal::create_string_type(),
                              H5Tclose);
             RMF_HDF5_CALL(H5Dread(d, mt, is, sp, H5P_DEFAULT, &c));
             if (c) {
               ret = std::string(c);
             }
             free(c);
           }
           , {
             RMF_UNUSED(d); RMF_UNUSED(is); RMF_UNUSED(s);
             RMF_NOT_IMPLEMENTED;
           }
           , {
             RMF_UNUSED(d); RMF_UNUSED(is);
             RMF_UNUSED(sp); RMF_UNUSED(sz);
             RMF_NOT_IMPLEMENTED;
           }
           , {
             RMF_UNUSED(a); RMF_UNUSED(v);
             RMF_NOT_IMPLEMENTED;
           }
           , {
             RMF_UNUSED(a); RMF_UNUSED(sz);
             RMF_NOT_IMPLEMENTED;
           }, false);

RMF_TRAITS_ONE(Char, Chars, char, 6, H5T_STD_I8LE,
               H5T_NATIVE_CHAR, H5T_NATIVE_CHAR, char, '\0',
               i == '\0',
               {
                 RMF_UNUSED(d); RMF_UNUSED(is); RMF_UNUSED(s);
                 RMF_UNUSED(v);
                 RMF_NOT_IMPLEMENTED;
               }, {
                 RMF_UNUSED(d); RMF_UNUSED(is); RMF_UNUSED(sp);
                 RMF_NOT_IMPLEMENTED;
                 ret = '\0';
               }
               , {
                 RMF_UNUSED(d); RMF_UNUSED(is); RMF_UNUSED(s);
                 RMF_NOT_IMPLEMENTED;
               }
               , {
                 RMF_UNUSED(d); RMF_UNUSED(is);
                 RMF_UNUSED(sp); RMF_UNUSED(sz);
                 RMF_NOT_IMPLEMENTED;
               }
               , {
                 RMF_HDF5_CALL(H5Awrite(a, H5T_NATIVE_CHAR, v.c_str()));
               }
               , {
                 std::vector<char> v(sz);
                 RMF_HDF5_CALL(H5Aread(a, H5T_NATIVE_CHAR, &v[0]));
                 ret = std::string(&v[0], v.size());
               }, false);

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_TYPES_H */
