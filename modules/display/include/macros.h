/**
 *  \file display/macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_MACROS_H
#define IMPDISPLAY_MACROS_H

//! Define information for an Writer object
/** Unfortunately, you have to make sure you include the code:
    \code
    if (get_stream_is_open()) {
    on_close();
    }
    \endcode
    in the destructor of your class.
 */
#define IMP_WRITER(version)                                             \
  virtual void add_geometry(IMP::display::Geometry *g);                 \
  void on_open(std::string name);                                       \
  virtual void on_close();                                              \
  virtual IMP::VersionInfo get_version_info() const {return version;}   \
  virtual void show(std::ostream &out=std::cout) const;

//! Since swig doesn't support using, this redefines the geometry methods
#define IMP_WRITER_ADD_GEOMETRY                                         \
  virtual void add_geometry(const IMP::display::Geometries &g) {        \
    Writer::add_geometry(g);                                            \
  }                                                                     \
  virtual void add_geometry(IMP::display::CompoundGeometry *cg) {       \
    Writer::add_geometry(cg);                                           \
  }                                                                     \
  virtual void add_geometry(const IMP::display::CompoundGeometries &g) { \
    Writer::add_geometry(g);                                            \
  }

//! Define information for an Geometry object
#define IMP_GEOMETRY(version)                                           \
  virtual unsigned int get_dimension() const;                           \
  virtual IMP::algebra::Vector3D get_vertex(unsigned int i) const;      \
  virtual IMP::VersionInfo get_version_info() const {return version;}   \
  virtual unsigned int get_number_of_vertices() const;                  \
  virtual void show(std::ostream &out=std::cout) const;

//! Define information for an Geometry object
#define IMP_COMPOUND_GEOMETRY(version)                                  \
  virtual IMP::display::Geometries get_geometry() const;                \
  virtual IMP::VersionInfo get_version_info() const {return version;}   \
  virtual void show(std::ostream &out=std::cout) const;


#endif /* IMPDISPLAY_MACROS_H */
