/**
 *  \file display/macros.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_MACROS_H
#define IMPDISPLAY_MACROS_H

//! Define information for an Writer object
#define IMP_WRITER(version)                                             \
  virtual void add_geometry(Geometry *g);                               \
  virtual void on_open(std::string name);                               \
  virtual void on_close();                                              \
  virtual VersionInfo get_version_info() const {return version;}        \
  virtual void show(std::ostream &out=std::cout) const;

//! Define information for an Geometry object
#define IMP_GEOMETRY(version)                                           \
  virtual unsigned int get_dimension() const;                           \
  virtual algebra::Vector3D get_vertex(unsigned int i) const;           \
  virtual VersionInfo get_version_info() const {return version;}        \
  virtual void show(std::ostream &out=std::cout) const;

//! Define information for an Geometry object
#define IMP_GEOMETRY_EXTRACTOR(version)                                 \
  virtual Geometries get_geometries(Particle *p) const;                 \
  virtual VersionInfo get_version_info() const {return version;}        \
  virtual void show(std::ostream &out=std::cout) const;


#endif /* IMPDISPLAY_MACROS_H */
