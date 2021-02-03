/**
 *  \file IMP/display/geometry_macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_GEOMETRY_MACROS_H
#define IMPDISPLAY_GEOMETRY_MACROS_H
#include "declare_Geometry.h"
#include <IMP/singleton_macros.h>
#include <IMP/pair_macros.h>

//! Define information for an Geometry object
#define IMP_DISPLAY_GEOMETRY_DEF(Name, Type)                     \
  Name::Name(std::string name) : display::Geometry(name) {}      \
  Name::Name(const Type &v) : display::Geometry(#Name), v_(v) {} \
  Name::Name(const Type &v, const Color &c)                      \
      : display::Geometry(c, #Name), v_(v) {}                    \
  Name::Name(const Type &v, const std::string n)                 \
      : display::Geometry(n), v_(v) {}                           \
  Name::Name(const Type &v, const Color &c, std::string n)       \
      : display::Geometry(c, n), v_(v) {}                        \
  display::Geometries Name::get_components() const {             \
    return display::Geometries(1, const_cast<Name *>(this));     \
  }                                                              \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#if defined(IMP_DOXYGEN) || defined(SWIG)
//! Define a geometric object using an IMP::algebra one
#define IMP_DISPLAY_GEOMETRY_DECL(Name, Names, Type)                      \
  /** Display a geometric object.*/                                       \
  class IMPDISPLAYEXPORT Name : public display::Geometry {                \
   public:                                                                \
    Name(const Type &v);                                                  \
    Name(const Type &v, const display::Color &c);                         \
    Name(const Type &v, const std::string n);                             \
    Name(const Type &v, const display::Color &c, std::string n);          \
    virtual const Type &get_geometry() const { return v_; }               \
    void set_geometry(const Type &v) { v_ = v; }                          \
    virtual IMP::display::Geometries get_components() const IMP_OVERRIDE; \
    IMP_OBJECT_METHODS(Name);                                             \
  };                                                                      \
  /** Create a Geometry with the passed primitive.*/                      \
  inline Name *create_geometry(const Type &t, std::string name = #Type +  \
                                                                 "%1"     \
                                                                 "%")

#define IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(Name, Names, Type)         \
  /** Display a compound geometric object.*/                              \
  class IMPDISPLAYEXPORT Name : public display::Geometry {                \
   public:                                                                \
    Name(const Type &v);                                                  \
    Name(const Type &v, const display::Color &c);                         \
    Name(const Type &v, const std::string n);                             \
    Name(const Type &v, const display::Color &c, std::string n);          \
    virtual const Type &get_geometry() const { return v_; }               \
    void set_geometry(const Type &v) { v_ = v; }                          \
    virtual IMP::display::Geometries get_components() const IMP_OVERRIDE; \
    IMP_OBJECT_METHODS(Name);                                             \
  };                                                                      \
  /** Create a Geometry with the passed primitive.*/                      \
  inline Name *create_geometry(const Type &t, std::string name = #Type +  \
                                                                 "%1"     \
                                                                 "%")

#else

//! Define a geometric object using an IMP::algebra one
#define IMP_DISPLAY_GEOMETRY_DECL(Name, Names, Type)                      \
  /** Display a geometric object.*/                                       \
  class IMPDISPLAYEXPORT Name : public display::Geometry {                \
    Type v_;                                                              \
                                                                          \
   public:                                                                \
    Name(std::string name);                                               \
    Name(const Type &v);                                                  \
    Name(const Type &v, const display::Color &c);                         \
    Name(const Type &v, const std::string n);                             \
    Name(const Type &v, const display::Color &c, std::string n);          \
    virtual const Type &get_geometry() const { return v_; }               \
    void set_geometry(const Type &v) { v_ = v; }                          \
    virtual IMP::display::Geometries get_components() const IMP_OVERRIDE; \
    IMP_OBJECT_METHODS(Name);                                             \
  };                                                                      \
  inline Name *create_geometry(                                           \
      const Type &t, std::string name = std::string(#Type) + "%1%") {     \
    return new Name(t, name);                                             \
  }                                                                       \
  IMP_OBJECTS(Name, Names)

#define IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(Name, Names, Type)         \
  /** Display a compound geometric object.*/                              \
  class IMPDISPLAYEXPORT Name : public display::Geometry {                \
    Type v_;                                                              \
                                                                          \
   public:                                                                \
    Name(std::string name);                                               \
    Name(const Type &v);                                                  \
    Name(const Type &v, const display::Color &c);                         \
    Name(const Type &v, const std::string n);                             \
    Name(const Type &v, const display::Color &c, std::string n);          \
    virtual const Type &get_geometry() const { return v_; }               \
    void set_geometry(const Type &v) { v_ = v; }                          \
    virtual IMP::display::Geometries get_components() const IMP_OVERRIDE; \
    IMP_OBJECT_METHODS(Name);                                             \
  };                                                                      \
  inline Name *create_geometry(                                           \
      const Type &t, std::string name = std::string(#Type) + "%1%") {     \
    return new Name(t, name);                                             \
  }                                                                       \
  IMP_OBJECTS(Name, Names)
#endif

#define IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DEF(Name, Type, decomp)   \
  Name::Name(std::string name) : display::Geometry(name) {}         \
  Name::Name(const Type &v) : display::Geometry(#Name), v_(v) {}    \
  Name::Name(const Type &v, const display::Color &c)                \
      : display::Geometry(c, #Name), v_(v) {}                       \
  Name::Name(const Type &v, const std::string n)                    \
      : display::Geometry(n), v_(v) {}                              \
  Name::Name(const Type &v, const display::Color &c, std::string n) \
      : display::Geometry(c, n), v_(v) {}                           \
  display::Geometries Name::get_components() const {                \
    display::Geometries ret;                                        \
    decomp;                                                         \
    return ret;                                                     \
  }                                                                 \
  IMP_REQUIRE_SEMICOLON_NAMESPACE

#define IMP_PARTICLE_GEOMETRY(Name, Decorator, action)                     \
  /** Display a particle.*/                                                \
  class Name##Geometry : public display::SingletonGeometry {               \
   public:                                                                 \
    Name##Geometry(Particle *p) : display::SingletonGeometry(p) {} \
    Name##Geometry(Decorator d) : display::SingletonGeometry(d) {}         \
    IMP_IMPLEMENT_INLINE(display::Geometries get_components() const, {     \
      display::Geometries ret;                                             \
      Decorator d(get_particle());                                         \
      action;                                                              \
      return ret;                                                          \
    });                                                                    \
    IMP_OBJECT_METHODS(Name##Geometry);                                    \
  };                                                                       \
  /** Display multiple particles.*/                                        \
  class Name##sGeometry : public display::SingletonsGeometry {             \
   public:                                                                 \
    Name##sGeometry(SingletonContainer *sc)                                \
        : display::SingletonsGeometry(sc) {}                               \
    display::Geometries get_components() const {                           \
      display::Geometries ret;                                             \
      IMP_FOREACH(ParticleIndex pi, get_container()->get_contents()) {     \
        Decorator d(get_container()->get_model(), pi);                     \
        action;                                                            \
      }                                                                    \
      return ret;                                                          \
    }                                                                      \
    IMP_OBJECT_METHODS(Name##sGeometry);                                   \
  }

#define IMP_PARTICLE_PAIR_GEOMETRY(Name, Decorator, action)            \
  /** Display a pair of particles.*/                                   \
  class Name##Geometry : public display::PairGeometry {                \
   public:                                                             \
    Name##Geometry(const ParticlePair &pp)                     \
        : display::PairGeometry(pp) {}                                 \
    display::Geometries get_components() const {                       \
      display::Geometries ret;                                         \
      Decorator d0(get_particle_pair()[0]);                            \
      Decorator d1(get_particle_pair()[1]);                            \
      action;                                                          \
      return ret;                                                      \
    }                                                                  \
    IMP_OBJECT_METHODS(Name##Geometry);                                \
  };                                                                   \
  /** Display multiple pairs of particles.*/                           \
  class Name##sGeometry : public display::PairsGeometry {              \
   public:                                                             \
    Name##sGeometry(PairContainer *sc) : display::PairsGeometry(sc) {} \
    display::Geometries get_components() const {                       \
      display::Geometries ret;                                         \
      IMP_FOREACH(ParticleIndexPair pip,                       \
                  get_container()->get_contents()) {                   \
        Decorator d0(get_container()->get_model(), pip[0]);            \
        Decorator d1(get_container()->get_model(), pip[1]);            \
        action;                                                        \
      }                                                                \
      return ret;                                                      \
    }                                                                  \
    IMP_OBJECT_METHODS(Name##sGeometry);                               \
  }

#endif /* IMPDISPLAY_GEOMETRY_MACROS_H */
