HDF5 {#hdf5}
====

We provide a simple set of classes for accessing core HDF5 functionality from C++. This was needed since
- The HDF5 C interface is not trivial to use properly, particularly in regards to data type conversions and resource management.
- It is very easy to use the C interface incorrectly without knowing it.
- The [standard HDF5 C++ interface](https://www.hdfgroup.org/solutions/hdf5/) doesn't really simplify use of the HDF5 library and doesn't make use of the features of C++.

The main classes provide access to:
- HDF5 files through RMF::HDF5::File and RMF::HDF5::ConstFile
- HDF5 groups through RMF::HDF5::Group and RMF::HDF5::ConstGroup
- HDF5 data sets through RMF::HDF5::DataSetD and RMF::HDF5::ConstDataSetD

(The `Const` variants are for read only files.)

These classes allow one to create a manipulate data sets containing floating point values, integers, strings, variable length arrays of floating point values and a few other types. Attributes on data sets of groups of those types can also be manipulated.

The top level classes are, in turn, derived from RMF::HDF5::Object, RMF::HDF5::ConstAttributes and RMF::HDF5MutableAttributes which provide access to general HDF5 per-object and attribute functionality (all objects in HDF5 can have arbitrary small pieces of data attached to them via attributes).

Not all of the functionality of the HDF5 C library is covered in C++. You can get the corresponding native handle from most objects with methods like RMF::HDF5::File::get_handle() and use that for other operations if needed. Alternatively, submit a patch to the library to add the missing functionality.
