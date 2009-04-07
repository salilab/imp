/**
 *  \file display/macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_MACROS_H
#define IMPDISPLAY_MACROS_H

//! Define information for an Writer object
/** This macro declares the methods on_open, on_close, add_geometry
    and show, and defines the destructor and get_version_info.
 */
#define IMP_WRITER(Name, version)                                       \
  virtual void add_geometry(IMP::display::Geometry *g);                 \
  virtual IMP::VersionInfo get_version_info() const {return version;}   \
  virtual void show(std::ostream &out=std::cout) const;                 \
  ~Name(){                                                              \
    if (get_stream_is_open()) {                                         \
    on_close();                                                         \
  }                                                                     \
  }                                                                     \
  IMP_NO_SWIG(protected:)                                               \
 virtual void on_open();                                                \
  virtual void on_close();                                              \



//! Since swig doesn't support using, this redefines the geometry methods
#define IMP_WRITER_ADD_GEOMETRY                                         \
  public:                                                               \
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
