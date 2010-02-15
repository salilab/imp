/**
 *  \file display/macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_MACROS_H
#define IMPDISPLAY_MACROS_H

//! Define information for an Writer object
/** This macro declares the methods on_open, on_close, add_geometry
    and show, and defines the destructor and get_version_info.
*/
#define IMP_WRITER(Name)                                                \
  Name(TextOutput of): Writer(of, #Name)                                \
  {}                                                                    \
  Name(): Writer(#Name){}                                               \
  IMP_OBJECT_INLINE(Name,,close());                                     \
protected:                                                              \
 using Writer::process;                                                 \
 virtual void on_open();                                                \
 virtual void on_close();                                               \


//! Define information for an Geometry object
#define IMP_GEOMETRY(Name)                              \
  IMP::display::Geometries get_components() const;      \
  IMP_OBJECT(Name)


//! Define a geometric object using an IMP::algebra one
#define IMP_DISPLAY_GEOMETRY_DECL(Name, Type)                           \
  class IMPDISPLAYEXPORT Name: public Type,                             \
                               public Geometry {                        \
  public:                                                               \
    Name(const Type &v);                                                \
    Name(const Type &v, const Color &c);                                \
    Name(const Type &v, const std::string n);                           \
    Name(const Type &v, const Color &c, std::string n);                 \
    IMP_GEOMETRY(Name);                      \
  };                                                                    \


#define IMP_DISPLAY_GEOMETRY_DEF(Name, Type)                            \
  Name::Name(const Type &v): Type(v), Geometry(#Name){}                 \
  Name::Name(const Type &v, const Color &c):                            \
    Type(v), Geometry(c, #Name) {}                                      \
  Name::Name(const Type &v, const std::string n):                       \
    Type(v), Geometry(n) {}                                             \
  Name::Name(const Type &v, const Color &c, std::string n):             \
    Type(v), Geometry(c,n) {}                                           \
  Geometries Name::get_components() const {                             \
    return Geometries(const_cast<Name*>(this));                         \
  }                                                                     \
  void Name::do_show(std::ostream &out) const {                         \
    out << #Name << "Geometry: " << static_cast<Type >(*this);          \
  }                                                                     \


#define IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(Name, Type)      \
  class IMPDISPLAYEXPORT Name: public Type,                     \
                               public Geometry {                \
  public:                                                       \
    Name(const Type &v);                                        \
    Name(const Type &v, const Color &c);                        \
    Name(const Type &v, const std::string n);                   \
    Name(const Type &v, const Color &c, std::string n);         \
    IMP_GEOMETRY(Name);                                         \
  };                                                            \


#define IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DEF(Name, Type, decomp)       \
  Name::Name(const Type &v): Type(v), Geometry(#Name){}                 \
  Name::Name(const Type &v, const Color &c):                            \
    Type(v), Geometry(c, #Name) {}                                      \
  Name::Name(const Type &v, const std::string n):                       \
    Type(v), Geometry(n) {}                                             \
  Name::Name(const Type &v, const Color &c, std::string n):             \
    Type(v), Geometry(c,n) {}                                           \
  void Name::do_show(std::ostream &out) const {                         \
    out << #Name << "Geometry: "                                        \
        << static_cast<Type>(*this);                                    \
  }                                                                     \
  Geometries Name::get_components() const {                             \
    Geometries ret;                                                     \
    decomp;                                                             \
    return ret;                                                         \
  }

#define IMP_PARTICLE_GEOMETRY(Name, Decorator, action)                  \
  class Name##Geometry: public SingletonGeometry {                      \
  public:                                                               \
  Name##Geometry(Decorator d): SingletonGeometry(d){}                   \
  Geometries get_components() const {                                   \
    Geometries ret;                                                     \
    Decorator d(get_particle());                                        \
    action;                                                             \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT_INLINE(Name##Geometry,                                     \
                    out <<  Decorator(get_particle())<< std::endl;,)    \
  };                                                                    \
  class Name##sGeometry: public SingletonsGeometry {                    \
  public:                                                               \
  Name##sGeometry(SingletonContainer* sc): SingletonsGeometry(sc){}     \
  Geometries get_components() const {                                   \
    Geometries ret;                                                     \
    for (unsigned int i=0;                                              \
         i< get_container()->get_number_of_particles();                 \
         ++i) {                                                         \
      Decorator d(get_container()->get_particle(i));                    \
      action;                                                           \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT_INLINE(Name##sGeometry,                                    \
                    out <<  get_container() << std::endl;,)             \
  };                                                                    \


#define IMP_PARTICLE_TRAITS_GEOMETRY(Name, Decorator, TraitsName,       \
                                     traits_name,action)                \
  class Name##Geometry: public SingletonGeometry {                      \
    TraitsName traits_;                                                 \
  public:                                                               \
  Name##Geometry(Decorator d): SingletonGeometry(d),                    \
    traits_(d.get_##traits_name()){}                                    \
  Geometries get_components() const {                                   \
    Geometries ret;                                                     \
    Decorator d(get_particle(), traits_);                               \
    action;                                                             \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT_INLINE(Name##Geometry,                                     \
                    out <<  Decorator(get_particle(), traits_)          \
                    << std::endl;,)                                     \
  };                                                                    \
  class Name##sGeometry: public SingletonsGeometry {                    \
    TraitsName traits_;                                                 \
  public:                                                               \
  Name##sGeometry(SingletonContainer* sc, TraitsName tr):               \
  SingletonsGeometry(sc), traits_(tr){}                                 \
  Geometries get_components() const {                                   \
    Geometries ret;                                                     \
    for (unsigned int i=0;                                              \
         i< get_container()->get_number_of_particles();                 \
         ++i) {                                                         \
      Decorator d(get_container()->get_particle(i), traits_);           \
      action;                                                           \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT_INLINE(Name##sGeometry,                                    \
                    out <<  get_container() << std::endl;,)             \
  };                                                                    \


#define IMP_PARTICLE_PAIR_GEOMETRY(Name, Decorator, action)             \
  class Name##Geometry: public PairGeometry {                           \
  public:                                                               \
  Name##Geometry(Decorator d0, Decorator d1):                           \
  PairGeometry(d0, d1){}                                                \
  Geometries get_components() const {                                   \
    Geometries ret;                                                     \
    Decorator d0(get_particle_pair()[0]);                               \
    Decorator d1(get_particle_pair()[1]);                               \
    action;                                                             \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT_INLINE(Name##Geometry,          \
                    out <<  Decorator(get_particle_pair()[0])           \
                    << " " << Decorator(get_particle_pair()[1])         \
                    << std::endl;,);                                    \
  };                                                                    \
  class Name##sGeometry: public PairsGeometry {                         \
  public:                                                               \
  Name##sGeometry(PairContainer* sc): PairsGeometry(sc){}               \
  Geometries get_components() const {                                   \
    Geometries ret;                                                     \
    for (unsigned int i=0;                                              \
         i< get_container()->get_number_of_particle_pairs();            \
         ++i) {                                                         \
      Decorator d0(get_container()->get_particle_pair(i)[0]);           \
      Decorator d1(get_container()->get_particle_pair(i)[1]);           \
      action;                                                           \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  IMP_OBJECT_INLINE(Name##sGeometry,                                    \
                    out <<  get_container() << std::endl;,)             \
  };

 #endif /* IMPDISPLAY_MACROS_H */
