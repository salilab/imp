/**
 *  \file RMF/decorators.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_DECORATORS_H
#define IMPLIBRMF_DECORATORS_H

#include "RMF_config.h"
#include "infrastructure_macros.h"
#include "NodeHandle.h"
#include "FileHandle.h"
#include "internal/utility.h"
#include "internal/lazy.h"
namespace RMF {

/** These particles has associated color information.
       \see Colored
       \see ColoredConstFactory
     */
    class ColoredConst {
    NodeConstHandle nh_;
    int frame_;
    friend class ColoredConstFactory;
    private:
    FloatKeys rgb_color_;
FloatKeys rgb_color_pf_;
    ColoredConst(NodeConstHandle nh,
                       int frame,
                  FloatKeys rgb_color,
FloatKeys rgb_color_pf):
       nh_(nh),
       frame_(frame),
       rgb_color_(rgb_color),
rgb_color_pf_(rgb_color_pf) {
    ;
    }
    public:
    Floats get_rgb_color() const {
             if (!rgb_color_.empty() && nh_.get_has_value(rgb_color_[0])) {
               return nh_.get_values(rgb_color_);
             } else {
               return nh_.get_values(rgb_color_pf_, frame_);
             }
           }
    IMP_RMF_SHOWABLE(ConstColored,
                     "ColoredConst "
                     << nh_.get_name());
    ~ColoredConst() {
    }
    };

    typedef vector<ColoredConst>
            ColoredConsts;

/** These particles has associated color information.

       \see ColoredConst
       \see ColoredFactory
     */
    class Colored {
    NodeHandle nh_;
    int frame_;
    friend class ColoredFactory;
    private:
    internal::FloatLazyKeys rgb_color_;
internal::FloatLazyKeys rgb_color_pf_;
    Colored(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKeys rgb_color,
internal::FloatLazyKeys rgb_color_pf):
       nh_(nh),
       frame_(frame),
       rgb_color_(rgb_color),
rgb_color_pf_(rgb_color_pf) {
    ;
    }
    public:
    Floats get_rgb_color() const {
             if (nh_.get_has_value(rgb_color_[0])) {
               return nh_.get_values(rgb_color_);
             } else {
               return nh_.get_values(rgb_color_pf_, frame_);
             }
           }
void set_rgb_color(const Floats &v) {
           if (frame_>=0) {
             nh_.set_values(rgb_color_pf_, v, frame_);
           } else {
             nh_.set_values(rgb_color_, v);
           }
        }
    IMP_RMF_SHOWABLE(ConstColored,
                     "Colored "
                     << nh_.get_name());
    ~Colored() {
    }
    };

    typedef vector<Colored>
            Coloreds;

/** Create decorators of type Colored.

       \see Colored
       \see ColoredConstFactory
    */
    class ColoredFactory {
    private:
    internal::FloatLazyKeys rgb_color_;
internal::FloatLazyKeys rgb_color_pf_;
    public:
    typedef FileHandle File;
    typedef Colored Decorator;
    ColoredFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
        Strings  rgb_color_names;
        rgb_color_names.push_back("rgb color red");
        rgb_color_names.push_back("rgb color green");
        rgb_color_names.push_back("rgb color blue");
      rgb_color_=internal::FloatLazyKeys(fh, cat,
                               rgb_color_names,
                               false);
      rgb_color_pf_=internal::FloatLazyKeys(fh, cat,
                               rgb_color_names,
                               true);
};
    }
    Colored get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Colored(nh, frame, rgb_color_,
rgb_color_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(rgb_color_pf_[0], frame)
                              || nh.get_has_value(rgb_color_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(rgb_color_[0])
                  || nh.get_has_value(rgb_color_pf_[0]))));
    }
    IMP_RMF_SHOWABLE(ColoredFactory,
                     "ColoredFactory");
    };

    typedef vector<ColoredFactory>
            ColoredFactories;

/** Create decorators of type Colored.

       \see ColoredConst
       \see ColoredFactory
    */
    class ColoredConstFactory {
    private:
    FloatKeys rgb_color_;
FloatKeys rgb_color_pf_;
    public:
    typedef FileConstHandle File;
    typedef ColoredConst Decorator;
    ColoredConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  rgb_color_names;
        rgb_color_names.push_back("rgb color red");
        rgb_color_names.push_back("rgb color green");
        rgb_color_names.push_back("rgb color blue");
      rgb_color_=(fh.get_has_key<FloatTraits>
                   (cat, rgb_color_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     rgb_color_names,
                                      false)
                              :FloatKeys());
      rgb_color_pf_=(fh.get_has_key<FloatTraits>
                   (cat, rgb_color_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     rgb_color_names,
                                      true)
                              :FloatKeys());
};
    }
    ColoredConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ColoredConst(nh, frame, rgb_color_,
rgb_color_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!rgb_color_pf_.empty()
                  && nh.get_has_value(rgb_color_pf_[0], frame))
                              || (!rgb_color_.empty()
                                 && nh.get_has_value(rgb_color_[0]))))
                     || (frame <0
                   &&  ((!rgb_color_.empty()
                       && nh.get_has_value(rgb_color_[0]))
                  || (!rgb_color_pf_.empty()
                     && nh.get_has_value(rgb_color_pf_[0])))));
    }
    IMP_RMF_SHOWABLE(ColoredConstFactory,
                     "ColoredConstFactory");
    };

    typedef vector<ColoredConstFactory>
            ColoredConstFactories;

/** These particles has associated coordinates and radius information.

       \see Particle
       \see ParticleConstFactory
     */
    class ParticleConst {
    NodeConstHandle nh_;
    int frame_;
    friend class ParticleConstFactory;
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
FloatKey mass_;
FloatKey mass_pf_;
    ParticleConst(NodeConstHandle nh,
                       int frame,
                  FloatKeys coordinates,
FloatKeys coordinates_pf,
FloatKey radius,
FloatKey radius_pf,
FloatKey mass,
FloatKey mass_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
mass_(mass),
mass_pf_(mass_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
             if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
Float get_mass() const {
  if (nh_.get_has_value(mass_)) {
   return nh_.get_value(mass_);
  } else {
   return nh_.get_value(mass_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstParticle,
                     "ParticleConst "
                     << nh_.get_name());
    ~ParticleConst() {
    }
    };

    typedef vector<ParticleConst>
            ParticleConsts;

/** These particles has associated coordinates and radius information.

       \see ParticleConst
       \see ParticleFactory
     */
    class Particle {
    NodeHandle nh_;
    int frame_;
    friend class ParticleFactory;
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::FloatLazyKey mass_;
internal::FloatLazyKey mass_pf_;
    Particle(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKeys coordinates,
internal::FloatLazyKeys coordinates_pf,
internal::FloatLazyKey radius,
internal::FloatLazyKey radius_pf,
internal::FloatLazyKey mass,
internal::FloatLazyKey mass_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
mass_(mass),
mass_pf_(mass_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
             if (nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
void set_coordinates(const Floats &v) {
           if (frame_>=0) {
             nh_.set_values(coordinates_pf_, v, frame_);
           } else {
             nh_.set_values(coordinates_, v);
           }
        }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
void set_radius(Float v) {
  if (frame_ >=0) {
    nh_.set_value(radius_pf_, v, frame_);
  } else {
    return nh_.set_value(radius_, v);
  }
}
Float get_mass() const {
  if (nh_.get_has_value(mass_)) {
   return nh_.get_value(mass_);
  } else {
   return nh_.get_value(mass_pf_, frame_);
  }
}
void set_mass(Float v) {
  if (frame_ >=0) {
    nh_.set_value(mass_pf_, v, frame_);
  } else {
    return nh_.set_value(mass_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstParticle,
                     "Particle "
                     << nh_.get_name());
    ~Particle() {
    }
    };

    typedef vector<Particle>
            Particles;

/** Create decorators of type Particle.

       \see Particle
       \see ParticleConstFactory
    */
    class ParticleFactory {
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::FloatLazyKey mass_;
internal::FloatLazyKey mass_pf_;
    public:
    typedef FileHandle File;
    typedef Particle Decorator;
    ParticleFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               true);
radius_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               false);
radius_pf_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               true);
mass_=internal::FloatLazyKey(fh, cat,
                               "mass",
                               false);
mass_pf_=internal::FloatLazyKey(fh, cat,
                               "mass",
                               true);
};
    }
    Particle get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Particle(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
mass_,
mass_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(mass_pf_, frame)
                          || nh.get_has_value(mass_)))
                || ( frame <0 && (nh.get_has_value(mass_)
                || nh.get_has_value(mass_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ParticleFactory,
                     "ParticleFactory");
    };

    typedef vector<ParticleFactory>
            ParticleFactories;

/** Create decorators of type Particle.

       \see ParticleConst
       \see ParticleFactory
    */
    class ParticleConstFactory {
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
FloatKey mass_;
FloatKey mass_pf_;
    public:
    typedef FileConstHandle File;
    typedef ParticleConst Decorator;
    ParticleConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatKeys());
      coordinates_pf_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatKeys());
radius_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      false)
                              :FloatKey());
radius_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      true)
                              :FloatKey());
mass_=(fh.get_has_key<FloatTraits>
                   (cat, "mass", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "mass",
                                      false)
                              :FloatKey());
mass_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "mass", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "mass",
                                      true)
                              :FloatKey());
};
    }
    ParticleConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ParticleConst(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
mass_,
mass_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(mass_pf_, frame)
                          || nh.get_has_value(mass_)))
                || ( frame <0 && (nh.get_has_value(mass_)
                || nh.get_has_value(mass_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ParticleConstFactory,
                     "ParticleConstFactory");
    };

    typedef vector<ParticleConstFactory>
            ParticleConstFactories;

/** These particles has associated coordinates and radius information.

       \see IntermediateParticle
       \see IntermediateParticleConstFactory
     */
    class IntermediateParticleConst {
    NodeConstHandle nh_;
    int frame_;
    friend class IntermediateParticleConstFactory;
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
    IntermediateParticleConst(NodeConstHandle nh,
                       int frame,
                  FloatKeys coordinates,
FloatKeys coordinates_pf,
FloatKey radius,
FloatKey radius_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
             if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstIntermediateParticle,
                     "IntermediateParticleConst "
                     << nh_.get_name());
    ~IntermediateParticleConst() {
    }
    };

    typedef vector<IntermediateParticleConst>
            IntermediateParticleConsts;

/** These particles has associated coordinates and radius information.

       \see IntermediateParticleConst
       \see IntermediateParticleFactory
     */
    class IntermediateParticle {
    NodeHandle nh_;
    int frame_;
    friend class IntermediateParticleFactory;
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
    IntermediateParticle(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKeys coordinates,
internal::FloatLazyKeys coordinates_pf,
internal::FloatLazyKey radius,
internal::FloatLazyKey radius_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
             if (nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
void set_coordinates(const Floats &v) {
           if (frame_>=0) {
             nh_.set_values(coordinates_pf_, v, frame_);
           } else {
             nh_.set_values(coordinates_, v);
           }
        }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
void set_radius(Float v) {
  if (frame_ >=0) {
    nh_.set_value(radius_pf_, v, frame_);
  } else {
    return nh_.set_value(radius_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstIntermediateParticle,
                     "IntermediateParticle "
                     << nh_.get_name());
    ~IntermediateParticle() {
    }
    };

    typedef vector<IntermediateParticle>
            IntermediateParticles;

/** Create decorators of type IntermediateParticle.

       \see IntermediateParticle
       \see IntermediateParticleConstFactory
    */
    class IntermediateParticleFactory {
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
    public:
    typedef FileHandle File;
    typedef IntermediateParticle Decorator;
    IntermediateParticleFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               true);
radius_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               false);
radius_pf_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               true);
};
    }
    IntermediateParticle get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return IntermediateParticle(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(IntermediateParticleFactory,
                     "IntermediateParticleFactory");
    };

    typedef vector<IntermediateParticleFactory>
            IntermediateParticleFactories;

/** Create decorators of type IntermediateParticle.

       \see IntermediateParticleConst
       \see IntermediateParticleFactory
    */
    class IntermediateParticleConstFactory {
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
    public:
    typedef FileConstHandle File;
    typedef IntermediateParticleConst Decorator;
    IntermediateParticleConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatKeys());
      coordinates_pf_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatKeys());
radius_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      false)
                              :FloatKey());
radius_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      true)
                              :FloatKey());
};
    }
    IntermediateParticleConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return IntermediateParticleConst(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(IntermediateParticleConstFactory,
                     "IntermediateParticleConstFactory");
    };

    typedef vector<IntermediateParticleConstFactory>
            IntermediateParticleConstFactories;

/** These particles has associated coordinates and orientation information.

       \see RigidParticle
       \see RigidParticleConstFactory
     */
    class RigidParticleConst {
    NodeConstHandle nh_;
    int frame_;
    friend class RigidParticleConstFactory;
    private:
    FloatKeys orientation_;
FloatKeys orientation_pf_;
FloatKeys coordinates_;
FloatKeys coordinates_pf_;
    RigidParticleConst(NodeConstHandle nh,
                       int frame,
                  FloatKeys orientation,
FloatKeys orientation_pf,
FloatKeys coordinates,
FloatKeys coordinates_pf):
       nh_(nh),
       frame_(frame),
       orientation_(orientation),
orientation_pf_(orientation_pf),
coordinates_(coordinates),
coordinates_pf_(coordinates_pf) {
    ;
    }
    public:
    Floats get_orientation() const {
             if (!orientation_.empty() && nh_.get_has_value(orientation_[0])) {
               return nh_.get_values(orientation_);
             } else {
               return nh_.get_values(orientation_pf_, frame_);
             }
           }
Floats get_coordinates() const {
             if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
    IMP_RMF_SHOWABLE(ConstRigidParticle,
                     "RigidParticleConst "
                     << nh_.get_name());
    ~RigidParticleConst() {
    }
    };

    typedef vector<RigidParticleConst>
            RigidParticleConsts;

/** These particles has associated coordinates and orientation information.

       \see RigidParticleConst
       \see RigidParticleFactory
     */
    class RigidParticle {
    NodeHandle nh_;
    int frame_;
    friend class RigidParticleFactory;
    private:
    internal::FloatLazyKeys orientation_;
internal::FloatLazyKeys orientation_pf_;
internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
    RigidParticle(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKeys orientation,
internal::FloatLazyKeys orientation_pf,
internal::FloatLazyKeys coordinates,
internal::FloatLazyKeys coordinates_pf):
       nh_(nh),
       frame_(frame),
       orientation_(orientation),
orientation_pf_(orientation_pf),
coordinates_(coordinates),
coordinates_pf_(coordinates_pf) {
    ;
    }
    public:
    Floats get_orientation() const {
             if (nh_.get_has_value(orientation_[0])) {
               return nh_.get_values(orientation_);
             } else {
               return nh_.get_values(orientation_pf_, frame_);
             }
           }
void set_orientation(const Floats &v) {
           if (frame_>=0) {
             nh_.set_values(orientation_pf_, v, frame_);
           } else {
             nh_.set_values(orientation_, v);
           }
        }
Floats get_coordinates() const {
             if (nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
void set_coordinates(const Floats &v) {
           if (frame_>=0) {
             nh_.set_values(coordinates_pf_, v, frame_);
           } else {
             nh_.set_values(coordinates_, v);
           }
        }
    IMP_RMF_SHOWABLE(ConstRigidParticle,
                     "RigidParticle "
                     << nh_.get_name());
    ~RigidParticle() {
    }
    };

    typedef vector<RigidParticle>
            RigidParticles;

/** Create decorators of type RigidParticle.

       \see RigidParticle
       \see RigidParticleConstFactory
    */
    class RigidParticleFactory {
    private:
    internal::FloatLazyKeys orientation_;
internal::FloatLazyKeys orientation_pf_;
internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
    public:
    typedef FileHandle File;
    typedef RigidParticle Decorator;
    RigidParticleFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
        Strings  orientation_names;
        orientation_names.push_back("orientation r");
        orientation_names.push_back("orientation i");
        orientation_names.push_back("orientation j");
        orientation_names.push_back("orientation k");
      orientation_=internal::FloatLazyKeys(fh, cat,
                               orientation_names,
                               false);
      orientation_pf_=internal::FloatLazyKeys(fh, cat,
                               orientation_names,
                               true);
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               true);
};
    }
    RigidParticle get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return RigidParticle(nh, frame, orientation_,
orientation_pf_,
coordinates_,
coordinates_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(orientation_pf_[0], frame)
                              || nh.get_has_value(orientation_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(orientation_[0])
                  || nh.get_has_value(orientation_pf_[0]))))
    && ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))));
    }
    IMP_RMF_SHOWABLE(RigidParticleFactory,
                     "RigidParticleFactory");
    };

    typedef vector<RigidParticleFactory>
            RigidParticleFactories;

/** Create decorators of type RigidParticle.

       \see RigidParticleConst
       \see RigidParticleFactory
    */
    class RigidParticleConstFactory {
    private:
    FloatKeys orientation_;
FloatKeys orientation_pf_;
FloatKeys coordinates_;
FloatKeys coordinates_pf_;
    public:
    typedef FileConstHandle File;
    typedef RigidParticleConst Decorator;
    RigidParticleConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  orientation_names;
        orientation_names.push_back("orientation r");
        orientation_names.push_back("orientation i");
        orientation_names.push_back("orientation j");
        orientation_names.push_back("orientation k");
      orientation_=(fh.get_has_key<FloatTraits>
                   (cat, orientation_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     orientation_names,
                                      false)
                              :FloatKeys());
      orientation_pf_=(fh.get_has_key<FloatTraits>
                   (cat, orientation_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     orientation_names,
                                      true)
                              :FloatKeys());
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatKeys());
      coordinates_pf_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatKeys());
};
    }
    RigidParticleConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return RigidParticleConst(nh, frame, orientation_,
orientation_pf_,
coordinates_,
coordinates_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!orientation_pf_.empty()
                  && nh.get_has_value(orientation_pf_[0], frame))
                              || (!orientation_.empty()
                                 && nh.get_has_value(orientation_[0]))))
                     || (frame <0
                   &&  ((!orientation_.empty()
                       && nh.get_has_value(orientation_[0]))
                  || (!orientation_pf_.empty()
                     && nh.get_has_value(orientation_pf_[0])))))
    && ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))));
    }
    IMP_RMF_SHOWABLE(RigidParticleConstFactory,
                     "RigidParticleConstFactory");
    };

    typedef vector<RigidParticleConstFactory>
            RigidParticleConstFactories;

/** A geometric ball.

       \see Ball
       \see BallConstFactory
     */
    class BallConst {
    NodeConstHandle nh_;
    int frame_;
    friend class BallConstFactory;
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
IndexKey type_;
IndexKey type_pf_;
    BallConst(NodeConstHandle nh,
                       int frame,
                  FloatKeys coordinates,
FloatKeys coordinates_pf,
FloatKey radius,
FloatKey radius_pf,
IndexKey type,
IndexKey type_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
type_(type),
type_pf_(type_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret(3);
       if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_[i]);
         }
       } else {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_pf_[i], frame_);
         }
       }
       return ret;
    }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstBall,
                     "BallConst "
                     << nh_.get_name());
    ~BallConst() {
    }
    };

    typedef vector<BallConst>
            BallConsts;

/** A geometric ball.

       \see BallConst
       \see BallFactory
     */
    class Ball {
    NodeHandle nh_;
    int frame_;
    friend class BallFactory;
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::IndexLazyKey type_;
internal::IndexLazyKey type_pf_;
    Ball(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKeys coordinates,
internal::FloatLazyKeys coordinates_pf,
internal::FloatLazyKey radius,
internal::FloatLazyKey radius_pf,
internal::IndexLazyKey type,
internal::IndexLazyKey type_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
type_(type),
type_pf_(type_pf) {
    nh.set_value(type_, 0);;
    }
    public:
    Floats get_coordinates() const {
       Floats ret(3);
       if (nh_.get_has_value(coordinates_[0])) {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_[i]);
         }
       } else {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_pf_[i], frame_);
         }
       }
       return ret;
    }
void set_coordinates(const Floats &v) {
         if (frame_>=0) {
           for (unsigned int i=0; i< 3; ++i) {
              nh_.set_value(coordinates_pf_[i], v[i], frame_);
           }
         } else {
           for (unsigned int i=0; i< 3; ++i) {
              nh_.set_value(coordinates_[i], v[i]);
           }
         }
      }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
void set_radius(Float v) {
  if (frame_ >=0) {
    nh_.set_value(radius_pf_, v, frame_);
  } else {
    return nh_.set_value(radius_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstBall,
                     "Ball "
                     << nh_.get_name());
    ~Ball() {
    }
    };

    typedef vector<Ball>
            Balls;

/** Create decorators of type Ball.

       \see Ball
       \see BallConstFactory
    */
    class BallFactory {
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::IndexLazyKey type_;
internal::IndexLazyKey type_pf_;
    public:
    typedef FileHandle File;
    typedef Ball Decorator;
    BallFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               true);
radius_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               false);
radius_pf_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               true);
type_=internal::IndexLazyKey(fh, cat,
                               "type",
                               false);
type_pf_=internal::IndexLazyKey(fh, cat,
                               "type",
                               true);
};
    }
    Ball get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Ball(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
type_,
type_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))))
    && nh.get_value(type_)==0;
    }
    IMP_RMF_SHOWABLE(BallFactory,
                     "BallFactory");
    };

    typedef vector<BallFactory>
            BallFactories;

/** Create decorators of type Ball.

       \see BallConst
       \see BallFactory
    */
    class BallConstFactory {
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
IndexKey type_;
IndexKey type_pf_;
    public:
    typedef FileConstHandle File;
    typedef BallConst Decorator;
    BallConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatKeys());
      coordinates_pf_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatKeys());
radius_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      false)
                              :FloatKey());
radius_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      true)
                              :FloatKey());
type_=(fh.get_has_key<IndexTraits>
                   (cat, "type", false)?
                   fh.get_key<IndexTraits>(cat,
                                     "type",
                                      false)
                              :IndexKey());
type_pf_=(fh.get_has_key<IndexTraits>
                   (cat, "type", true)?
                   fh.get_key<IndexTraits>(cat,
                                     "type",
                                      true)
                              :IndexKey());
};
    }
    BallConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return BallConst(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
type_,
type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))))
    && nh.get_value(type_)==0;
    }
    IMP_RMF_SHOWABLE(BallConstFactory,
                     "BallConstFactory");
    };

    typedef vector<BallConstFactory>
            BallConstFactories;

/** A geometric cylinder.

       \see Cylinder
       \see CylinderConstFactory
     */
    class CylinderConst {
    NodeConstHandle nh_;
    int frame_;
    friend class CylinderConstFactory;
    private:
    FloatsKeys coordinates_;
FloatsKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
IndexKey type_;
IndexKey type_pf_;
    CylinderConst(NodeConstHandle nh,
                       int frame,
                  FloatsKeys coordinates,
FloatsKeys coordinates_pf,
FloatKey radius,
FloatKey radius_pf,
IndexKey type,
IndexKey type_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
type_(type),
type_pf_(type_pf) {
    ;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret(3);
       if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_[i]);
         }
       } else {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_pf_[i], frame_);
         }
       }
       return ret;
    }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstCylinder,
                     "CylinderConst "
                     << nh_.get_name());
    ~CylinderConst() {
    }
    };

    typedef vector<CylinderConst>
            CylinderConsts;

/** A geometric cylinder.

       \see CylinderConst
       \see CylinderFactory
     */
    class Cylinder {
    NodeHandle nh_;
    int frame_;
    friend class CylinderFactory;
    private:
    internal::FloatsLazyKeys coordinates_;
internal::FloatsLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::IndexLazyKey type_;
internal::IndexLazyKey type_pf_;
    Cylinder(NodeHandle nh,
                       int frame,
                  internal::FloatsLazyKeys coordinates,
internal::FloatsLazyKeys coordinates_pf,
internal::FloatLazyKey radius,
internal::FloatLazyKey radius_pf,
internal::IndexLazyKey type,
internal::IndexLazyKey type_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
type_(type),
type_pf_(type_pf) {
    nh.set_value(type_, 1);;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret(3);
       if (nh_.get_has_value(coordinates_[0])) {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_[i]);
         }
       } else {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_pf_[i], frame_);
         }
       }
       return ret;
    }
void set_coordinates(const FloatsList &v) {
         if (frame_>=0) {
           for (unsigned int i=0; i< 3; ++i) {
              nh_.set_value(coordinates_pf_[i], v[i], frame_);
           }
         } else {
           for (unsigned int i=0; i< 3; ++i) {
              nh_.set_value(coordinates_[i], v[i]);
           }
         }
      }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
void set_radius(Float v) {
  if (frame_ >=0) {
    nh_.set_value(radius_pf_, v, frame_);
  } else {
    return nh_.set_value(radius_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstCylinder,
                     "Cylinder "
                     << nh_.get_name());
    ~Cylinder() {
    }
    };

    typedef vector<Cylinder>
            Cylinders;

/** Create decorators of type Cylinder.

       \see Cylinder
       \see CylinderConstFactory
    */
    class CylinderFactory {
    private:
    internal::FloatsLazyKeys coordinates_;
internal::FloatsLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::IndexLazyKey type_;
internal::IndexLazyKey type_pf_;
    public:
    typedef FileHandle File;
    typedef Cylinder Decorator;
    CylinderFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian xs");
        coordinates_names.push_back("cartesian ys");
        coordinates_names.push_back("cartesian zs");
      coordinates_=internal::FloatsLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatsLazyKeys(fh, cat,
                               coordinates_names,
                               true);
radius_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               false);
radius_pf_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               true);
type_=internal::IndexLazyKey(fh, cat,
                               "type",
                               false);
type_pf_=internal::IndexLazyKey(fh, cat,
                               "type",
                               true);
};
    }
    Cylinder get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Cylinder(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
type_,
type_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))))
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(CylinderFactory,
                     "CylinderFactory");
    };

    typedef vector<CylinderFactory>
            CylinderFactories;

/** Create decorators of type Cylinder.

       \see CylinderConst
       \see CylinderFactory
    */
    class CylinderConstFactory {
    private:
    FloatsKeys coordinates_;
FloatsKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
IndexKey type_;
IndexKey type_pf_;
    public:
    typedef FileConstHandle File;
    typedef CylinderConst Decorator;
    CylinderConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian xs");
        coordinates_names.push_back("cartesian ys");
        coordinates_names.push_back("cartesian zs");
      coordinates_=(fh.get_has_key<FloatsTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatsTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatsKeys());
      coordinates_pf_=(fh.get_has_key<FloatsTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatsTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatsKeys());
radius_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      false)
                              :FloatKey());
radius_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      true)
                              :FloatKey());
type_=(fh.get_has_key<IndexTraits>
                   (cat, "type", false)?
                   fh.get_key<IndexTraits>(cat,
                                     "type",
                                      false)
                              :IndexKey());
type_pf_=(fh.get_has_key<IndexTraits>
                   (cat, "type", true)?
                   fh.get_key<IndexTraits>(cat,
                                     "type",
                                      true)
                              :IndexKey());
};
    }
    CylinderConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return CylinderConst(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
type_,
type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))))
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(CylinderConstFactory,
                     "CylinderConstFactory");
    };

    typedef vector<CylinderConstFactory>
            CylinderConstFactories;

/** A geometric line setgment.

       \see Segment
       \see SegmentConstFactory
     */
    class SegmentConst {
    NodeConstHandle nh_;
    int frame_;
    friend class SegmentConstFactory;
    private:
    FloatsKeys coordinates_;
FloatsKeys coordinates_pf_;
IndexKey type_;
IndexKey type_pf_;
    SegmentConst(NodeConstHandle nh,
                       int frame,
                  FloatsKeys coordinates,
FloatsKeys coordinates_pf,
IndexKey type,
IndexKey type_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
type_(type),
type_pf_(type_pf) {
    ;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret(3);
       if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_[i]);
         }
       } else {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_pf_[i], frame_);
         }
       }
       return ret;
    }
    IMP_RMF_SHOWABLE(ConstSegment,
                     "SegmentConst "
                     << nh_.get_name());
    ~SegmentConst() {
    }
    };

    typedef vector<SegmentConst>
            SegmentConsts;

/** A geometric line setgment.

       \see SegmentConst
       \see SegmentFactory
     */
    class Segment {
    NodeHandle nh_;
    int frame_;
    friend class SegmentFactory;
    private:
    internal::FloatsLazyKeys coordinates_;
internal::FloatsLazyKeys coordinates_pf_;
internal::IndexLazyKey type_;
internal::IndexLazyKey type_pf_;
    Segment(NodeHandle nh,
                       int frame,
                  internal::FloatsLazyKeys coordinates,
internal::FloatsLazyKeys coordinates_pf,
internal::IndexLazyKey type,
internal::IndexLazyKey type_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
type_(type),
type_pf_(type_pf) {
    nh.set_value(type_, 1);;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret(3);
       if (nh_.get_has_value(coordinates_[0])) {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_[i]);
         }
       } else {
         for (unsigned int i=0; i< 3; ++i) {
          ret[i]=nh_.get_value(coordinates_pf_[i], frame_);
         }
       }
       return ret;
    }
void set_coordinates(const FloatsList &v) {
         if (frame_>=0) {
           for (unsigned int i=0; i< 3; ++i) {
              nh_.set_value(coordinates_pf_[i], v[i], frame_);
           }
         } else {
           for (unsigned int i=0; i< 3; ++i) {
              nh_.set_value(coordinates_[i], v[i]);
           }
         }
      }
    IMP_RMF_SHOWABLE(ConstSegment,
                     "Segment "
                     << nh_.get_name());
    ~Segment() {
    }
    };

    typedef vector<Segment>
            Segments;

/** Create decorators of type Segment.

       \see Segment
       \see SegmentConstFactory
    */
    class SegmentFactory {
    private:
    internal::FloatsLazyKeys coordinates_;
internal::FloatsLazyKeys coordinates_pf_;
internal::IndexLazyKey type_;
internal::IndexLazyKey type_pf_;
    public:
    typedef FileHandle File;
    typedef Segment Decorator;
    SegmentFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian xs");
        coordinates_names.push_back("cartesian ys");
        coordinates_names.push_back("cartesian zs");
      coordinates_=internal::FloatsLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatsLazyKeys(fh, cat,
                               coordinates_names,
                               true);
type_=internal::IndexLazyKey(fh, cat,
                               "type",
                               false);
type_pf_=internal::IndexLazyKey(fh, cat,
                               "type",
                               true);
};
    }
    Segment get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Segment(nh, frame, coordinates_,
coordinates_pf_,
type_,
type_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))))
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))))
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(SegmentFactory,
                     "SegmentFactory");
    };

    typedef vector<SegmentFactory>
            SegmentFactories;

/** Create decorators of type Segment.

       \see SegmentConst
       \see SegmentFactory
    */
    class SegmentConstFactory {
    private:
    FloatsKeys coordinates_;
FloatsKeys coordinates_pf_;
IndexKey type_;
IndexKey type_pf_;
    public:
    typedef FileConstHandle File;
    typedef SegmentConst Decorator;
    SegmentConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian xs");
        coordinates_names.push_back("cartesian ys");
        coordinates_names.push_back("cartesian zs");
      coordinates_=(fh.get_has_key<FloatsTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatsTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatsKeys());
      coordinates_pf_=(fh.get_has_key<FloatsTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatsTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatsKeys());
type_=(fh.get_has_key<IndexTraits>
                   (cat, "type", false)?
                   fh.get_key<IndexTraits>(cat,
                                     "type",
                                      false)
                              :IndexKey());
type_pf_=(fh.get_has_key<IndexTraits>
                   (cat, "type", true)?
                   fh.get_key<IndexTraits>(cat,
                                     "type",
                                      true)
                              :IndexKey());
};
    }
    SegmentConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return SegmentConst(nh, frame, coordinates_,
coordinates_pf_,
type_,
type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))))
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))))
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(SegmentConstFactory,
                     "SegmentConstFactory");
    };

    typedef vector<SegmentConstFactory>
            SegmentConstFactories;

/** Information regarding a publication.

       \see JournalArticle
       \see JournalArticleConstFactory
     */
    class JournalArticleConst {
    NodeConstHandle nh_;
    int frame_;
    friend class JournalArticleConstFactory;
    private:
    StringKey title_;
StringKey title_pf_;
StringKey journal_;
StringKey journal_pf_;
StringKey pubmed_id_;
StringKey pubmed_id_pf_;
IntKey year_;
IntKey year_pf_;
StringsKey authors_;
StringsKey authors_pf_;
    JournalArticleConst(NodeConstHandle nh,
                       int frame,
                  StringKey title,
StringKey title_pf,
StringKey journal,
StringKey journal_pf,
StringKey pubmed_id,
StringKey pubmed_id_pf,
IntKey year,
IntKey year_pf,
StringsKey authors,
StringsKey authors_pf):
       nh_(nh),
       frame_(frame),
       title_(title),
title_pf_(title_pf),
journal_(journal),
journal_pf_(journal_pf),
pubmed_id_(pubmed_id),
pubmed_id_pf_(pubmed_id_pf),
year_(year),
year_pf_(year_pf),
authors_(authors),
authors_pf_(authors_pf) {
    ;
    }
    public:
    String get_title() const {
  if (nh_.get_has_value(title_)) {
   return nh_.get_value(title_);
  } else {
   return nh_.get_value(title_pf_, frame_);
  }
}
String get_journal() const {
  if (nh_.get_has_value(journal_)) {
   return nh_.get_value(journal_);
  } else {
   return nh_.get_value(journal_pf_, frame_);
  }
}
String get_pubmed_id() const {
  if (nh_.get_has_value(pubmed_id_)) {
   return nh_.get_value(pubmed_id_);
  } else {
   return nh_.get_value(pubmed_id_pf_, frame_);
  }
}
Int get_year() const {
  if (nh_.get_has_value(year_)) {
   return nh_.get_value(year_);
  } else {
   return nh_.get_value(year_pf_, frame_);
  }
}
Strings get_authors() const {
  if (nh_.get_has_value(authors_)) {
   return nh_.get_value(authors_);
  } else {
   return nh_.get_value(authors_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstJournalArticle,
                     "JournalArticleConst "
                     << nh_.get_name());
    ~JournalArticleConst() {
    }
    };

    typedef vector<JournalArticleConst>
            JournalArticleConsts;

/** Information regarding a publication.

       \see JournalArticleConst
       \see JournalArticleFactory
     */
    class JournalArticle {
    NodeHandle nh_;
    int frame_;
    friend class JournalArticleFactory;
    private:
    internal::StringLazyKey title_;
internal::StringLazyKey title_pf_;
internal::StringLazyKey journal_;
internal::StringLazyKey journal_pf_;
internal::StringLazyKey pubmed_id_;
internal::StringLazyKey pubmed_id_pf_;
internal::IntLazyKey year_;
internal::IntLazyKey year_pf_;
internal::StringsLazyKey authors_;
internal::StringsLazyKey authors_pf_;
    JournalArticle(NodeHandle nh,
                       int frame,
                  internal::StringLazyKey title,
internal::StringLazyKey title_pf,
internal::StringLazyKey journal,
internal::StringLazyKey journal_pf,
internal::StringLazyKey pubmed_id,
internal::StringLazyKey pubmed_id_pf,
internal::IntLazyKey year,
internal::IntLazyKey year_pf,
internal::StringsLazyKey authors,
internal::StringsLazyKey authors_pf):
       nh_(nh),
       frame_(frame),
       title_(title),
title_pf_(title_pf),
journal_(journal),
journal_pf_(journal_pf),
pubmed_id_(pubmed_id),
pubmed_id_pf_(pubmed_id_pf),
year_(year),
year_pf_(year_pf),
authors_(authors),
authors_pf_(authors_pf) {
    ;
    }
    public:
    String get_title() const {
  if (nh_.get_has_value(title_)) {
   return nh_.get_value(title_);
  } else {
   return nh_.get_value(title_pf_, frame_);
  }
}
void set_title(String v) {
  if (frame_ >=0) {
    nh_.set_value(title_pf_, v, frame_);
  } else {
    return nh_.set_value(title_, v);
  }
}
String get_journal() const {
  if (nh_.get_has_value(journal_)) {
   return nh_.get_value(journal_);
  } else {
   return nh_.get_value(journal_pf_, frame_);
  }
}
void set_journal(String v) {
  if (frame_ >=0) {
    nh_.set_value(journal_pf_, v, frame_);
  } else {
    return nh_.set_value(journal_, v);
  }
}
String get_pubmed_id() const {
  if (nh_.get_has_value(pubmed_id_)) {
   return nh_.get_value(pubmed_id_);
  } else {
   return nh_.get_value(pubmed_id_pf_, frame_);
  }
}
void set_pubmed_id(String v) {
  if (frame_ >=0) {
    nh_.set_value(pubmed_id_pf_, v, frame_);
  } else {
    return nh_.set_value(pubmed_id_, v);
  }
}
Int get_year() const {
  if (nh_.get_has_value(year_)) {
   return nh_.get_value(year_);
  } else {
   return nh_.get_value(year_pf_, frame_);
  }
}
void set_year(Int v) {
  if (frame_ >=0) {
    nh_.set_value(year_pf_, v, frame_);
  } else {
    return nh_.set_value(year_, v);
  }
}
Strings get_authors() const {
  if (nh_.get_has_value(authors_)) {
   return nh_.get_value(authors_);
  } else {
   return nh_.get_value(authors_pf_, frame_);
  }
}
void set_authors(Strings v) {
  if (frame_ >=0) {
    nh_.set_value(authors_pf_, v, frame_);
  } else {
    return nh_.set_value(authors_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstJournalArticle,
                     "JournalArticle "
                     << nh_.get_name());
    ~JournalArticle() {
    }
    };

    typedef vector<JournalArticle>
            JournalArticles;

/** Create decorators of type JournalArticle.

       \see JournalArticle
       \see JournalArticleConstFactory
    */
    class JournalArticleFactory {
    private:
    internal::StringLazyKey title_;
internal::StringLazyKey title_pf_;
internal::StringLazyKey journal_;
internal::StringLazyKey journal_pf_;
internal::StringLazyKey pubmed_id_;
internal::StringLazyKey pubmed_id_pf_;
internal::IntLazyKey year_;
internal::IntLazyKey year_pf_;
internal::StringsLazyKey authors_;
internal::StringsLazyKey authors_pf_;
    public:
    typedef FileHandle File;
    typedef JournalArticle Decorator;
    JournalArticleFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "publication");
title_=internal::StringLazyKey(fh, cat,
                               "title",
                               false);
title_pf_=internal::StringLazyKey(fh, cat,
                               "title",
                               true);
journal_=internal::StringLazyKey(fh, cat,
                               "journal",
                               false);
journal_pf_=internal::StringLazyKey(fh, cat,
                               "journal",
                               true);
pubmed_id_=internal::StringLazyKey(fh, cat,
                               "pubmed id",
                               false);
pubmed_id_pf_=internal::StringLazyKey(fh, cat,
                               "pubmed id",
                               true);
year_=internal::IntLazyKey(fh, cat,
                               "year",
                               false);
year_pf_=internal::IntLazyKey(fh, cat,
                               "year",
                               true);
authors_=internal::StringsLazyKey(fh, cat,
                               "authors",
                               false);
authors_pf_=internal::StringsLazyKey(fh, cat,
                               "authors",
                               true);
};
    }
    JournalArticle get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return JournalArticle(nh, frame, title_,
title_pf_,
journal_,
journal_pf_,
pubmed_id_,
pubmed_id_pf_,
year_,
year_pf_,
authors_,
authors_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(title_pf_, frame)
                          || nh.get_has_value(title_)))
                || ( frame <0 && (nh.get_has_value(title_)
                || nh.get_has_value(title_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(journal_pf_, frame)
                          || nh.get_has_value(journal_)))
                || ( frame <0 && (nh.get_has_value(journal_)
                || nh.get_has_value(journal_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(pubmed_id_pf_, frame)
                          || nh.get_has_value(pubmed_id_)))
                || ( frame <0 && (nh.get_has_value(pubmed_id_)
                || nh.get_has_value(pubmed_id_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(year_pf_, frame)
                          || nh.get_has_value(year_)))
                || ( frame <0 && (nh.get_has_value(year_)
                || nh.get_has_value(year_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(authors_pf_, frame)
                          || nh.get_has_value(authors_)))
                || ( frame <0 && (nh.get_has_value(authors_)
                || nh.get_has_value(authors_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(JournalArticleFactory,
                     "JournalArticleFactory");
    };

    typedef vector<JournalArticleFactory>
            JournalArticleFactories;

/** Create decorators of type JournalArticle.

       \see JournalArticleConst
       \see JournalArticleFactory
    */
    class JournalArticleConstFactory {
    private:
    StringKey title_;
StringKey title_pf_;
StringKey journal_;
StringKey journal_pf_;
StringKey pubmed_id_;
StringKey pubmed_id_pf_;
IntKey year_;
IntKey year_pf_;
StringsKey authors_;
StringsKey authors_pf_;
    public:
    typedef FileConstHandle File;
    typedef JournalArticleConst Decorator;
    JournalArticleConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("publication");
title_=(fh.get_has_key<StringTraits>
                   (cat, "title", false)?
                   fh.get_key<StringTraits>(cat,
                                     "title",
                                      false)
                              :StringKey());
title_pf_=(fh.get_has_key<StringTraits>
                   (cat, "title", true)?
                   fh.get_key<StringTraits>(cat,
                                     "title",
                                      true)
                              :StringKey());
journal_=(fh.get_has_key<StringTraits>
                   (cat, "journal", false)?
                   fh.get_key<StringTraits>(cat,
                                     "journal",
                                      false)
                              :StringKey());
journal_pf_=(fh.get_has_key<StringTraits>
                   (cat, "journal", true)?
                   fh.get_key<StringTraits>(cat,
                                     "journal",
                                      true)
                              :StringKey());
pubmed_id_=(fh.get_has_key<StringTraits>
                   (cat, "pubmed id", false)?
                   fh.get_key<StringTraits>(cat,
                                     "pubmed id",
                                      false)
                              :StringKey());
pubmed_id_pf_=(fh.get_has_key<StringTraits>
                   (cat, "pubmed id", true)?
                   fh.get_key<StringTraits>(cat,
                                     "pubmed id",
                                      true)
                              :StringKey());
year_=(fh.get_has_key<IntTraits>
                   (cat, "year", false)?
                   fh.get_key<IntTraits>(cat,
                                     "year",
                                      false)
                              :IntKey());
year_pf_=(fh.get_has_key<IntTraits>
                   (cat, "year", true)?
                   fh.get_key<IntTraits>(cat,
                                     "year",
                                      true)
                              :IntKey());
authors_=(fh.get_has_key<StringsTraits>
                   (cat, "authors", false)?
                   fh.get_key<StringsTraits>(cat,
                                     "authors",
                                      false)
                              :StringsKey());
authors_pf_=(fh.get_has_key<StringsTraits>
                   (cat, "authors", true)?
                   fh.get_key<StringsTraits>(cat,
                                     "authors",
                                      true)
                              :StringsKey());
};
    }
    JournalArticleConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return JournalArticleConst(nh, frame, title_,
title_pf_,
journal_,
journal_pf_,
pubmed_id_,
pubmed_id_pf_,
year_,
year_pf_,
authors_,
authors_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(title_pf_, frame)
                          || nh.get_has_value(title_)))
                || ( frame <0 && (nh.get_has_value(title_)
                || nh.get_has_value(title_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(journal_pf_, frame)
                          || nh.get_has_value(journal_)))
                || ( frame <0 && (nh.get_has_value(journal_)
                || nh.get_has_value(journal_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(pubmed_id_pf_, frame)
                          || nh.get_has_value(pubmed_id_)))
                || ( frame <0 && (nh.get_has_value(pubmed_id_)
                || nh.get_has_value(pubmed_id_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(year_pf_, frame)
                          || nh.get_has_value(year_)))
                || ( frame <0 && (nh.get_has_value(year_)
                || nh.get_has_value(year_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(authors_pf_, frame)
                          || nh.get_has_value(authors_)))
                || ( frame <0 && (nh.get_has_value(authors_)
                || nh.get_has_value(authors_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(JournalArticleConstFactory,
                     "JournalArticleConstFactory");
    };

    typedef vector<JournalArticleConstFactory>
            JournalArticleConstFactories;

/** Information regarding a residue.

       \see Residue
       \see ResidueConstFactory
     */
    class ResidueConst {
    NodeConstHandle nh_;
    int frame_;
    friend class ResidueConstFactory;
    private:
    boost::array<IntKey,2> index_;
StringKey type_;
StringKey type_pf_;
    ResidueConst(NodeConstHandle nh,
                       int frame,
                  boost::array<IntKey, 2> index,
StringKey type,
StringKey type_pf):
       nh_(nh),
       frame_(frame),
       index_(index),
type_(type),
type_pf_(type_pf) {
    ;
    }
    public:
    Int get_index() const {
  return nh_.get_value(index_[0], frame_);
}
String get_type() const {
  if (nh_.get_has_value(type_)) {
   return nh_.get_value(type_);
  } else {
   return nh_.get_value(type_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstResidue,
                     "ResidueConst "
                     << nh_.get_name());
    ~ResidueConst() {
    }
    };

    typedef vector<ResidueConst>
            ResidueConsts;

/** Information regarding a residue.

       \see ResidueConst
       \see ResidueFactory
     */
    class Residue {
    NodeHandle nh_;
    int frame_;
    friend class ResidueFactory;
    private:
    boost::array<internal::IntLazyKey,2> index_;
internal::StringLazyKey type_;
internal::StringLazyKey type_pf_;
    Residue(NodeHandle nh,
                       int frame,
                  boost::array<internal::IntLazyKey, 2> index,
internal::StringLazyKey type,
internal::StringLazyKey type_pf):
       nh_(nh),
       frame_(frame),
       index_(index),
type_(type),
type_pf_(type_pf) {
    ;
    }
    public:
    Int get_index() const {
  return nh_.get_value(index_[0], frame_);
}
void set_index(Int v) {
   nh_.set_value(index_[0], v, frame_);
   nh_.set_value(index_[1], v, frame_);
}
String get_type() const {
  if (nh_.get_has_value(type_)) {
   return nh_.get_value(type_);
  } else {
   return nh_.get_value(type_pf_, frame_);
  }
}
void set_type(String v) {
  if (frame_ >=0) {
    nh_.set_value(type_pf_, v, frame_);
  } else {
    return nh_.set_value(type_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstResidue,
                     "Residue "
                     << nh_.get_name());
    ~Residue() {
    }
    };

    typedef vector<Residue>
            Residues;

/** Create decorators of type Residue.

       \see Residue
       \see ResidueConstFactory
    */
    class ResidueFactory {
    private:
    boost::array<internal::IntLazyKey,2> index_;
internal::StringLazyKey type_;
internal::StringLazyKey type_pf_;
    public:
    typedef FileHandle File;
    typedef Residue Decorator;
    ResidueFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
index_[0]=internal::IntLazyKey(fh, cat,
                               "first residue index",
                               false);
index_[1]=internal::IntLazyKey(fh, cat,
                               "last residue index",
                               false);
type_=internal::StringLazyKey(fh, cat,
                               "residue type",
                               false);
type_pf_=internal::StringLazyKey(fh, cat,
                               "residue type",
                               true);
};
    }
    Residue get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Residue(nh, frame, index_,
type_,
type_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return nh.get_has_value(index_[0], frame)
  && nh.get_has_value(index_[1], frame)
  && nh.get_value(index_[0], frame)
   ==nh.get_value(index_[1], frame)
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ResidueFactory,
                     "ResidueFactory");
    };

    typedef vector<ResidueFactory>
            ResidueFactories;

/** Create decorators of type Residue.

       \see ResidueConst
       \see ResidueFactory
    */
    class ResidueConstFactory {
    private:
    boost::array<IntKey,2> index_;
StringKey type_;
StringKey type_pf_;
    public:
    typedef FileConstHandle File;
    typedef ResidueConst Decorator;
    ResidueConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
index_[0]=(fh.get_has_key<IntTraits>
                   (cat, "first residue index", false)?
                   fh.get_key<IntTraits>(cat,
                                     "first residue index",
                                      false)
                              :IntKey());
index_[1]=(fh.get_has_key<IntTraits>
                   (cat, "last residue index", false)?
                   fh.get_key<IntTraits>(cat,
                                     "last residue index",
                                      false)
                              :IntKey());
type_=(fh.get_has_key<StringTraits>
                   (cat, "residue type", false)?
                   fh.get_key<StringTraits>(cat,
                                     "residue type",
                                      false)
                              :StringKey());
type_pf_=(fh.get_has_key<StringTraits>
                   (cat, "residue type", true)?
                   fh.get_key<StringTraits>(cat,
                                     "residue type",
                                      true)
                              :StringKey());
};
    }
    ResidueConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ResidueConst(nh, frame, index_,
type_,
type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return nh.get_has_value(index_[0], frame)
  && nh.get_has_value(index_[1], frame)
  && nh.get_value(index_[0], frame)
   ==nh.get_value(index_[1], frame)
    && ((frame >=0 && (nh.get_has_value(type_pf_, frame)
                          || nh.get_has_value(type_)))
                || ( frame <0 && (nh.get_has_value(type_)
                || nh.get_has_value(type_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ResidueConstFactory,
                     "ResidueConstFactory");
    };

    typedef vector<ResidueConstFactory>
            ResidueConstFactories;

/** Information regarding an atom.

       \see Atom
       \see AtomConstFactory
     */
    class AtomConst {
    NodeConstHandle nh_;
    int frame_;
    friend class AtomConstFactory;
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
FloatKey mass_;
FloatKey mass_pf_;
IndexKey element_;
IndexKey element_pf_;
    AtomConst(NodeConstHandle nh,
                       int frame,
                  FloatKeys coordinates,
FloatKeys coordinates_pf,
FloatKey radius,
FloatKey radius_pf,
FloatKey mass,
FloatKey mass_pf,
IndexKey element,
IndexKey element_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
mass_(mass),
mass_pf_(mass_pf),
element_(element),
element_pf_(element_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
             if (!coordinates_.empty() && nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
Float get_mass() const {
  if (nh_.get_has_value(mass_)) {
   return nh_.get_value(mass_);
  } else {
   return nh_.get_value(mass_pf_, frame_);
  }
}
Index get_element() const {
  if (nh_.get_has_value(element_)) {
   return nh_.get_value(element_);
  } else {
   return nh_.get_value(element_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstAtom,
                     "AtomConst "
                     << nh_.get_name());
    ~AtomConst() {
    }
    };

    typedef vector<AtomConst>
            AtomConsts;

/** Information regarding an atom.

       \see AtomConst
       \see AtomFactory
     */
    class Atom {
    NodeHandle nh_;
    int frame_;
    friend class AtomFactory;
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::FloatLazyKey mass_;
internal::FloatLazyKey mass_pf_;
internal::IndexLazyKey element_;
internal::IndexLazyKey element_pf_;
    Atom(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKeys coordinates,
internal::FloatLazyKeys coordinates_pf,
internal::FloatLazyKey radius,
internal::FloatLazyKey radius_pf,
internal::FloatLazyKey mass,
internal::FloatLazyKey mass_pf,
internal::IndexLazyKey element,
internal::IndexLazyKey element_pf):
       nh_(nh),
       frame_(frame),
       coordinates_(coordinates),
coordinates_pf_(coordinates_pf),
radius_(radius),
radius_pf_(radius_pf),
mass_(mass),
mass_pf_(mass_pf),
element_(element),
element_pf_(element_pf) {
    ;
    }
    public:
    Floats get_coordinates() const {
             if (nh_.get_has_value(coordinates_[0])) {
               return nh_.get_values(coordinates_);
             } else {
               return nh_.get_values(coordinates_pf_, frame_);
             }
           }
void set_coordinates(const Floats &v) {
           if (frame_>=0) {
             nh_.set_values(coordinates_pf_, v, frame_);
           } else {
             nh_.set_values(coordinates_, v);
           }
        }
Float get_radius() const {
  if (nh_.get_has_value(radius_)) {
   return nh_.get_value(radius_);
  } else {
   return nh_.get_value(radius_pf_, frame_);
  }
}
void set_radius(Float v) {
  if (frame_ >=0) {
    nh_.set_value(radius_pf_, v, frame_);
  } else {
    return nh_.set_value(radius_, v);
  }
}
Float get_mass() const {
  if (nh_.get_has_value(mass_)) {
   return nh_.get_value(mass_);
  } else {
   return nh_.get_value(mass_pf_, frame_);
  }
}
void set_mass(Float v) {
  if (frame_ >=0) {
    nh_.set_value(mass_pf_, v, frame_);
  } else {
    return nh_.set_value(mass_, v);
  }
}
Index get_element() const {
  if (nh_.get_has_value(element_)) {
   return nh_.get_value(element_);
  } else {
   return nh_.get_value(element_pf_, frame_);
  }
}
void set_element(Index v) {
  if (frame_ >=0) {
    nh_.set_value(element_pf_, v, frame_);
  } else {
    return nh_.set_value(element_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstAtom,
                     "Atom "
                     << nh_.get_name());
    ~Atom() {
    }
    };

    typedef vector<Atom>
            Atoms;

/** Create decorators of type Atom.

       \see Atom
       \see AtomConstFactory
    */
    class AtomFactory {
    private:
    internal::FloatLazyKeys coordinates_;
internal::FloatLazyKeys coordinates_pf_;
internal::FloatLazyKey radius_;
internal::FloatLazyKey radius_pf_;
internal::FloatLazyKey mass_;
internal::FloatLazyKey mass_pf_;
internal::IndexLazyKey element_;
internal::IndexLazyKey element_pf_;
    public:
    typedef FileHandle File;
    typedef Atom Decorator;
    AtomFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               false);
      coordinates_pf_=internal::FloatLazyKeys(fh, cat,
                               coordinates_names,
                               true);
radius_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               false);
radius_pf_=internal::FloatLazyKey(fh, cat,
                               "radius",
                               true);
mass_=internal::FloatLazyKey(fh, cat,
                               "mass",
                               false);
mass_pf_=internal::FloatLazyKey(fh, cat,
                               "mass",
                               true);
element_=internal::IndexLazyKey(fh, cat,
                               "element",
                               false);
element_pf_=internal::IndexLazyKey(fh, cat,
                               "element",
                               true);
};
    }
    Atom get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Atom(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
mass_,
mass_pf_,
element_,
element_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0
                          && (nh.get_has_value(coordinates_pf_[0], frame)
                              || nh.get_has_value(coordinates_[0])))
                     || (frame <0
                   &&  (nh.get_has_value(coordinates_[0])
                  || nh.get_has_value(coordinates_pf_[0]))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(mass_pf_, frame)
                          || nh.get_has_value(mass_)))
                || ( frame <0 && (nh.get_has_value(mass_)
                || nh.get_has_value(mass_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(element_pf_, frame)
                          || nh.get_has_value(element_)))
                || ( frame <0 && (nh.get_has_value(element_)
                || nh.get_has_value(element_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(AtomFactory,
                     "AtomFactory");
    };

    typedef vector<AtomFactory>
            AtomFactories;

/** Create decorators of type Atom.

       \see AtomConst
       \see AtomFactory
    */
    class AtomConstFactory {
    private:
    FloatKeys coordinates_;
FloatKeys coordinates_pf_;
FloatKey radius_;
FloatKey radius_pf_;
FloatKey mass_;
FloatKey mass_pf_;
IndexKey element_;
IndexKey element_pf_;
    public:
    typedef FileConstHandle File;
    typedef AtomConst Decorator;
    AtomConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
      coordinates_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], false)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      false)
                              :FloatKeys());
      coordinates_pf_=(fh.get_has_key<FloatTraits>
                   (cat, coordinates_names[0], true)?
                   fh.get_keys<FloatTraits>(cat,
                                     coordinates_names,
                                      true)
                              :FloatKeys());
radius_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      false)
                              :FloatKey());
radius_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "radius", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "radius",
                                      true)
                              :FloatKey());
mass_=(fh.get_has_key<FloatTraits>
                   (cat, "mass", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "mass",
                                      false)
                              :FloatKey());
mass_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "mass", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "mass",
                                      true)
                              :FloatKey());
element_=(fh.get_has_key<IndexTraits>
                   (cat, "element", false)?
                   fh.get_key<IndexTraits>(cat,
                                     "element",
                                      false)
                              :IndexKey());
element_pf_=(fh.get_has_key<IndexTraits>
                   (cat, "element", true)?
                   fh.get_key<IndexTraits>(cat,
                                     "element",
                                      true)
                              :IndexKey());
};
    }
    AtomConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return AtomConst(nh, frame, coordinates_,
coordinates_pf_,
radius_,
radius_pf_,
mass_,
mass_pf_,
element_,
element_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0
                  && ((!coordinates_pf_.empty()
                  && nh.get_has_value(coordinates_pf_[0], frame))
                              || (!coordinates_.empty()
                                 && nh.get_has_value(coordinates_[0]))))
                     || (frame <0
                   &&  ((!coordinates_.empty()
                       && nh.get_has_value(coordinates_[0]))
                  || (!coordinates_pf_.empty()
                     && nh.get_has_value(coordinates_pf_[0])))))
    && ((frame >=0 && (nh.get_has_value(radius_pf_, frame)
                          || nh.get_has_value(radius_)))
                || ( frame <0 && (nh.get_has_value(radius_)
                || nh.get_has_value(radius_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(mass_pf_, frame)
                          || nh.get_has_value(mass_)))
                || ( frame <0 && (nh.get_has_value(mass_)
                || nh.get_has_value(mass_pf_, 0))))
    && ((frame >=0 && (nh.get_has_value(element_pf_, frame)
                          || nh.get_has_value(element_)))
                || ( frame <0 && (nh.get_has_value(element_)
                || nh.get_has_value(element_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(AtomConstFactory,
                     "AtomConstFactory");
    };

    typedef vector<AtomConstFactory>
            AtomConstFactories;

/** Information regarding a chain.

       \see Chain
       \see ChainConstFactory
     */
    class ChainConst {
    NodeConstHandle nh_;
    int frame_;
    friend class ChainConstFactory;
    private:
    IndexKey chain_id_;
IndexKey chain_id_pf_;
    ChainConst(NodeConstHandle nh,
                       int frame,
                  IndexKey chain_id,
IndexKey chain_id_pf):
       nh_(nh),
       frame_(frame),
       chain_id_(chain_id),
chain_id_pf_(chain_id_pf) {
    ;
    }
    public:
    Index get_chain_id() const {
  if (nh_.get_has_value(chain_id_)) {
   return nh_.get_value(chain_id_);
  } else {
   return nh_.get_value(chain_id_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstChain,
                     "ChainConst "
                     << nh_.get_name());
    ~ChainConst() {
    }
    };

    typedef vector<ChainConst>
            ChainConsts;

/** Information regarding a chain.

       \see ChainConst
       \see ChainFactory
     */
    class Chain {
    NodeHandle nh_;
    int frame_;
    friend class ChainFactory;
    private:
    internal::IndexLazyKey chain_id_;
internal::IndexLazyKey chain_id_pf_;
    Chain(NodeHandle nh,
                       int frame,
                  internal::IndexLazyKey chain_id,
internal::IndexLazyKey chain_id_pf):
       nh_(nh),
       frame_(frame),
       chain_id_(chain_id),
chain_id_pf_(chain_id_pf) {
    ;
    }
    public:
    Index get_chain_id() const {
  if (nh_.get_has_value(chain_id_)) {
   return nh_.get_value(chain_id_);
  } else {
   return nh_.get_value(chain_id_pf_, frame_);
  }
}
void set_chain_id(Index v) {
  if (frame_ >=0) {
    nh_.set_value(chain_id_pf_, v, frame_);
  } else {
    return nh_.set_value(chain_id_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstChain,
                     "Chain "
                     << nh_.get_name());
    ~Chain() {
    }
    };

    typedef vector<Chain>
            Chains;

/** Create decorators of type Chain.

       \see Chain
       \see ChainConstFactory
    */
    class ChainFactory {
    private:
    internal::IndexLazyKey chain_id_;
internal::IndexLazyKey chain_id_pf_;
    public:
    typedef FileHandle File;
    typedef Chain Decorator;
    ChainFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
chain_id_=internal::IndexLazyKey(fh, cat,
                               "chain id",
                               false);
chain_id_pf_=internal::IndexLazyKey(fh, cat,
                               "chain id",
                               true);
};
    }
    Chain get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Chain(nh, frame, chain_id_,
chain_id_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(chain_id_pf_, frame)
                          || nh.get_has_value(chain_id_)))
                || ( frame <0 && (nh.get_has_value(chain_id_)
                || nh.get_has_value(chain_id_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ChainFactory,
                     "ChainFactory");
    };

    typedef vector<ChainFactory>
            ChainFactories;

/** Create decorators of type Chain.

       \see ChainConst
       \see ChainFactory
    */
    class ChainConstFactory {
    private:
    IndexKey chain_id_;
IndexKey chain_id_pf_;
    public:
    typedef FileConstHandle File;
    typedef ChainConst Decorator;
    ChainConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
chain_id_=(fh.get_has_key<IndexTraits>
                   (cat, "chain id", false)?
                   fh.get_key<IndexTraits>(cat,
                                     "chain id",
                                      false)
                              :IndexKey());
chain_id_pf_=(fh.get_has_key<IndexTraits>
                   (cat, "chain id", true)?
                   fh.get_key<IndexTraits>(cat,
                                     "chain id",
                                      true)
                              :IndexKey());
};
    }
    ChainConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ChainConst(nh, frame, chain_id_,
chain_id_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(chain_id_pf_, frame)
                          || nh.get_has_value(chain_id_)))
                || ( frame <0 && (nh.get_has_value(chain_id_)
                || nh.get_has_value(chain_id_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ChainConstFactory,
                     "ChainConstFactory");
    };

    typedef vector<ChainConstFactory>
            ChainConstFactories;

/** Information regarding a fragment of a molecule.

       \see Domain
       \see DomainConstFactory
     */
    class DomainConst {
    NodeConstHandle nh_;
    int frame_;
    friend class DomainConstFactory;
    private:
    boost::array<IntKey,2> indexes_;
    DomainConst(NodeConstHandle nh,
                       int frame,
                  boost::array<IntKey, 2> indexes):
       nh_(nh),
       frame_(frame),
       indexes_(indexes) {
    ;
    }
    public:
    IntRange get_indexes() const {
  return std::make_pair(nh_.get_value(indexes_[0], frame_),
                        nh_.get_value(indexes_[1], frame_));
}
    IMP_RMF_SHOWABLE(ConstDomain,
                     "DomainConst "
                     << nh_.get_name());
    ~DomainConst() {
    }
    };

    typedef vector<DomainConst>
            DomainConsts;

/** Information regarding a fragment of a molecule.

       \see DomainConst
       \see DomainFactory
     */
    class Domain {
    NodeHandle nh_;
    int frame_;
    friend class DomainFactory;
    private:
    boost::array<internal::IntLazyKey,2> indexes_;
    Domain(NodeHandle nh,
                       int frame,
                  boost::array<internal::IntLazyKey, 2> indexes):
       nh_(nh),
       frame_(frame),
       indexes_(indexes) {
    ;
    }
    public:
    IntRange get_indexes() const {
  return std::make_pair(nh_.get_value(indexes_[0], frame_),
                        nh_.get_value(indexes_[1], frame_));
}
void set_indexes(Int v0, Int v1) {
   nh_.set_value(indexes_[0], v0, frame_);
   nh_.set_value(indexes_[1], v1, frame_);
}
    IMP_RMF_SHOWABLE(ConstDomain,
                     "Domain "
                     << nh_.get_name());
    ~Domain() {
    }
    };

    typedef vector<Domain>
            Domains;

/** Create decorators of type Domain.

       \see Domain
       \see DomainConstFactory
    */
    class DomainFactory {
    private:
    boost::array<internal::IntLazyKey,2> indexes_;
    public:
    typedef FileHandle File;
    typedef Domain Decorator;
    DomainFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
indexes_[0]=internal::IntLazyKey(fh, cat,
                               "first residue index",
                               false);
indexes_[1]=internal::IntLazyKey(fh, cat,
                               "last residue index",
                               false);
};
    }
    Domain get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Domain(nh, frame, indexes_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return nh.get_has_value(indexes_[0], frame)
  && nh.get_has_value(indexes_[1], frame)
  && nh.get_value(indexes_[0], frame)
   <nh.get_value(indexes_[1], frame);
    }
    IMP_RMF_SHOWABLE(DomainFactory,
                     "DomainFactory");
    };

    typedef vector<DomainFactory>
            DomainFactories;

/** Create decorators of type Domain.

       \see DomainConst
       \see DomainFactory
    */
    class DomainConstFactory {
    private:
    boost::array<IntKey,2> indexes_;
    public:
    typedef FileConstHandle File;
    typedef DomainConst Decorator;
    DomainConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
indexes_[0]=(fh.get_has_key<IntTraits>
                   (cat, "first residue index", false)?
                   fh.get_key<IntTraits>(cat,
                                     "first residue index",
                                      false)
                              :IntKey());
indexes_[1]=(fh.get_has_key<IntTraits>
                   (cat, "last residue index", false)?
                   fh.get_key<IntTraits>(cat,
                                     "last residue index",
                                      false)
                              :IntKey());
};
    }
    DomainConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return DomainConst(nh, frame, indexes_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return nh.get_has_value(indexes_[0], frame)
  && nh.get_has_value(indexes_[1], frame)
  && nh.get_value(indexes_[0], frame)
   <nh.get_value(indexes_[1], frame);
    }
    IMP_RMF_SHOWABLE(DomainConstFactory,
                     "DomainConstFactory");
    };

    typedef vector<DomainConstFactory>
            DomainConstFactories;

/** Information regarding a copy of a molecule.

       \see Copy
       \see CopyConstFactory
     */
    class CopyConst {
    NodeConstHandle nh_;
    int frame_;
    friend class CopyConstFactory;
    private:
    IndexKey copy_index_;
IndexKey copy_index_pf_;
    CopyConst(NodeConstHandle nh,
                       int frame,
                  IndexKey copy_index,
IndexKey copy_index_pf):
       nh_(nh),
       frame_(frame),
       copy_index_(copy_index),
copy_index_pf_(copy_index_pf) {
    ;
    }
    public:
    Index get_copy_index() const {
  if (nh_.get_has_value(copy_index_)) {
   return nh_.get_value(copy_index_);
  } else {
   return nh_.get_value(copy_index_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstCopy,
                     "CopyConst "
                     << nh_.get_name());
    ~CopyConst() {
    }
    };

    typedef vector<CopyConst>
            CopyConsts;

/** Information regarding a copy of a molecule.

       \see CopyConst
       \see CopyFactory
     */
    class Copy {
    NodeHandle nh_;
    int frame_;
    friend class CopyFactory;
    private:
    internal::IndexLazyKey copy_index_;
internal::IndexLazyKey copy_index_pf_;
    Copy(NodeHandle nh,
                       int frame,
                  internal::IndexLazyKey copy_index,
internal::IndexLazyKey copy_index_pf):
       nh_(nh),
       frame_(frame),
       copy_index_(copy_index),
copy_index_pf_(copy_index_pf) {
    ;
    }
    public:
    Index get_copy_index() const {
  if (nh_.get_has_value(copy_index_)) {
   return nh_.get_value(copy_index_);
  } else {
   return nh_.get_value(copy_index_pf_, frame_);
  }
}
void set_copy_index(Index v) {
  if (frame_ >=0) {
    nh_.set_value(copy_index_pf_, v, frame_);
  } else {
    return nh_.set_value(copy_index_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstCopy,
                     "Copy "
                     << nh_.get_name());
    ~Copy() {
    }
    };

    typedef vector<Copy>
            Copys;

/** Create decorators of type Copy.

       \see Copy
       \see CopyConstFactory
    */
    class CopyFactory {
    private:
    internal::IndexLazyKey copy_index_;
internal::IndexLazyKey copy_index_pf_;
    public:
    typedef FileHandle File;
    typedef Copy Decorator;
    CopyFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
copy_index_=internal::IndexLazyKey(fh, cat,
                               "copy index",
                               false);
copy_index_pf_=internal::IndexLazyKey(fh, cat,
                               "copy index",
                               true);
};
    }
    Copy get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Copy(nh, frame, copy_index_,
copy_index_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(copy_index_pf_, frame)
                          || nh.get_has_value(copy_index_)))
                || ( frame <0 && (nh.get_has_value(copy_index_)
                || nh.get_has_value(copy_index_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(CopyFactory,
                     "CopyFactory");
    };

    typedef vector<CopyFactory>
            CopyFactories;

/** Create decorators of type Copy.

       \see CopyConst
       \see CopyFactory
    */
    class CopyConstFactory {
    private:
    IndexKey copy_index_;
IndexKey copy_index_pf_;
    public:
    typedef FileConstHandle File;
    typedef CopyConst Decorator;
    CopyConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
copy_index_=(fh.get_has_key<IndexTraits>
                   (cat, "copy index", false)?
                   fh.get_key<IndexTraits>(cat,
                                     "copy index",
                                      false)
                              :IndexKey());
copy_index_pf_=(fh.get_has_key<IndexTraits>
                   (cat, "copy index", true)?
                   fh.get_key<IndexTraits>(cat,
                                     "copy index",
                                      true)
                              :IndexKey());
};
    }
    CopyConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return CopyConst(nh, frame, copy_index_,
copy_index_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(copy_index_pf_, frame)
                          || nh.get_has_value(copy_index_)))
                || ( frame <0 && (nh.get_has_value(copy_index_)
                || nh.get_has_value(copy_index_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(CopyConstFactory,
                     "CopyConstFactory");
    };

    typedef vector<CopyConstFactory>
            CopyConstFactories;

/** Information regarding diffusion coefficients.

       \see Diffuser
       \see DiffuserConstFactory
     */
    class DiffuserConst {
    NodeConstHandle nh_;
    int frame_;
    friend class DiffuserConstFactory;
    private:
    FloatKey diffusion_coefficient_;
FloatKey diffusion_coefficient_pf_;
    DiffuserConst(NodeConstHandle nh,
                       int frame,
                  FloatKey diffusion_coefficient,
FloatKey diffusion_coefficient_pf):
       nh_(nh),
       frame_(frame),
       diffusion_coefficient_(diffusion_coefficient),
diffusion_coefficient_pf_(diffusion_coefficient_pf) {
    ;
    }
    public:
    Float get_diffusion_coefficient() const {
  if (nh_.get_has_value(diffusion_coefficient_)) {
   return nh_.get_value(diffusion_coefficient_);
  } else {
   return nh_.get_value(diffusion_coefficient_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstDiffuser,
                     "DiffuserConst "
                     << nh_.get_name());
    ~DiffuserConst() {
    }
    };

    typedef vector<DiffuserConst>
            DiffuserConsts;

/** Information regarding diffusion coefficients.

       \see DiffuserConst
       \see DiffuserFactory
     */
    class Diffuser {
    NodeHandle nh_;
    int frame_;
    friend class DiffuserFactory;
    private:
    internal::FloatLazyKey diffusion_coefficient_;
internal::FloatLazyKey diffusion_coefficient_pf_;
    Diffuser(NodeHandle nh,
                       int frame,
                  internal::FloatLazyKey diffusion_coefficient,
internal::FloatLazyKey diffusion_coefficient_pf):
       nh_(nh),
       frame_(frame),
       diffusion_coefficient_(diffusion_coefficient),
diffusion_coefficient_pf_(diffusion_coefficient_pf) {
    ;
    }
    public:
    Float get_diffusion_coefficient() const {
  if (nh_.get_has_value(diffusion_coefficient_)) {
   return nh_.get_value(diffusion_coefficient_);
  } else {
   return nh_.get_value(diffusion_coefficient_pf_, frame_);
  }
}
void set_diffusion_coefficient(Float v) {
  if (frame_ >=0) {
    nh_.set_value(diffusion_coefficient_pf_, v, frame_);
  } else {
    return nh_.set_value(diffusion_coefficient_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstDiffuser,
                     "Diffuser "
                     << nh_.get_name());
    ~Diffuser() {
    }
    };

    typedef vector<Diffuser>
            Diffusers;

/** Create decorators of type Diffuser.

       \see Diffuser
       \see DiffuserConstFactory
    */
    class DiffuserFactory {
    private:
    internal::FloatLazyKey diffusion_coefficient_;
internal::FloatLazyKey diffusion_coefficient_pf_;
    public:
    typedef FileHandle File;
    typedef Diffuser Decorator;
    DiffuserFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
diffusion_coefficient_=internal::FloatLazyKey(fh, cat,
                               "diffusion coefficient",
                               false);
diffusion_coefficient_pf_=internal::FloatLazyKey(fh, cat,
                               "diffusion coefficient",
                               true);
};
    }
    Diffuser get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Diffuser(nh, frame, diffusion_coefficient_,
diffusion_coefficient_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(diffusion_coefficient_pf_, frame)
                          || nh.get_has_value(diffusion_coefficient_)))
                || ( frame <0 && (nh.get_has_value(diffusion_coefficient_)
                || nh.get_has_value(diffusion_coefficient_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(DiffuserFactory,
                     "DiffuserFactory");
    };

    typedef vector<DiffuserFactory>
            DiffuserFactories;

/** Create decorators of type Diffuser.

       \see DiffuserConst
       \see DiffuserFactory
    */
    class DiffuserConstFactory {
    private:
    FloatKey diffusion_coefficient_;
FloatKey diffusion_coefficient_pf_;
    public:
    typedef FileConstHandle File;
    typedef DiffuserConst Decorator;
    DiffuserConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
diffusion_coefficient_=(fh.get_has_key<FloatTraits>
                   (cat, "diffusion coefficient", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "diffusion coefficient",
                                      false)
                              :FloatKey());
diffusion_coefficient_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "diffusion coefficient", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "diffusion coefficient",
                                      true)
                              :FloatKey());
};
    }
    DiffuserConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return DiffuserConst(nh, frame, diffusion_coefficient_,
diffusion_coefficient_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(diffusion_coefficient_pf_, frame)
                          || nh.get_has_value(diffusion_coefficient_)))
                || ( frame <0 && (nh.get_has_value(diffusion_coefficient_)
                || nh.get_has_value(diffusion_coefficient_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(DiffuserConstFactory,
                     "DiffuserConstFactory");
    };

    typedef vector<DiffuserConstFactory>
            DiffuserConstFactories;

/** A numeric tag for keeping track of types of molecules.

       \see Typed
       \see TypedConstFactory
     */
    class TypedConst {
    NodeConstHandle nh_;
    int frame_;
    friend class TypedConstFactory;
    private:
    StringKey type_name_;
StringKey type_name_pf_;
    TypedConst(NodeConstHandle nh,
                       int frame,
                  StringKey type_name,
StringKey type_name_pf):
       nh_(nh),
       frame_(frame),
       type_name_(type_name),
type_name_pf_(type_name_pf) {
    ;
    }
    public:
    String get_type_name() const {
  if (nh_.get_has_value(type_name_)) {
   return nh_.get_value(type_name_);
  } else {
   return nh_.get_value(type_name_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstTyped,
                     "TypedConst "
                     << nh_.get_name());
    ~TypedConst() {
    }
    };

    typedef vector<TypedConst>
            TypedConsts;

/** A numeric tag for keeping track of types of molecules.

       \see TypedConst
       \see TypedFactory
     */
    class Typed {
    NodeHandle nh_;
    int frame_;
    friend class TypedFactory;
    private:
    internal::StringLazyKey type_name_;
internal::StringLazyKey type_name_pf_;
    Typed(NodeHandle nh,
                       int frame,
                  internal::StringLazyKey type_name,
internal::StringLazyKey type_name_pf):
       nh_(nh),
       frame_(frame),
       type_name_(type_name),
type_name_pf_(type_name_pf) {
    ;
    }
    public:
    String get_type_name() const {
  if (nh_.get_has_value(type_name_)) {
   return nh_.get_value(type_name_);
  } else {
   return nh_.get_value(type_name_pf_, frame_);
  }
}
void set_type_name(String v) {
  if (frame_ >=0) {
    nh_.set_value(type_name_pf_, v, frame_);
  } else {
    return nh_.set_value(type_name_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstTyped,
                     "Typed "
                     << nh_.get_name());
    ~Typed() {
    }
    };

    typedef vector<Typed>
            Typeds;

/** Create decorators of type Typed.

       \see Typed
       \see TypedConstFactory
    */
    class TypedFactory {
    private:
    internal::StringLazyKey type_name_;
internal::StringLazyKey type_name_pf_;
    public:
    typedef FileHandle File;
    typedef Typed Decorator;
    TypedFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
type_name_=internal::StringLazyKey(fh, cat,
                               "type name",
                               false);
type_name_pf_=internal::StringLazyKey(fh, cat,
                               "type name",
                               true);
};
    }
    Typed get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Typed(nh, frame, type_name_,
type_name_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(type_name_pf_, frame)
                          || nh.get_has_value(type_name_)))
                || ( frame <0 && (nh.get_has_value(type_name_)
                || nh.get_has_value(type_name_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(TypedFactory,
                     "TypedFactory");
    };

    typedef vector<TypedFactory>
            TypedFactories;

/** Create decorators of type Typed.

       \see TypedConst
       \see TypedFactory
    */
    class TypedConstFactory {
    private:
    StringKey type_name_;
StringKey type_name_pf_;
    public:
    typedef FileConstHandle File;
    typedef TypedConst Decorator;
    TypedConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
type_name_=(fh.get_has_key<StringTraits>
                   (cat, "type name", false)?
                   fh.get_key<StringTraits>(cat,
                                     "type name",
                                      false)
                              :StringKey());
type_name_pf_=(fh.get_has_key<StringTraits>
                   (cat, "type name", true)?
                   fh.get_key<StringTraits>(cat,
                                     "type name",
                                      true)
                              :StringKey());
};
    }
    TypedConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return TypedConst(nh, frame, type_name_,
type_name_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(type_name_pf_, frame)
                          || nh.get_has_value(type_name_)))
                || ( frame <0 && (nh.get_has_value(type_name_)
                || nh.get_has_value(type_name_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(TypedConstFactory,
                     "TypedConstFactory");
    };

    typedef vector<TypedConstFactory>
            TypedConstFactories;

/** Store a static reference to another node.

       \see StaticAlias
       \see StaticAliasConstFactory
     */
    class StaticAliasConst {
    NodeConstHandle nh_;
    int frame_;
    friend class StaticAliasConstFactory;
    private:
    NodeIDKey aliased_;
NodeIDKey aliased_pf_;
    StaticAliasConst(NodeConstHandle nh,
                       int frame,
                  NodeIDKey aliased,
NodeIDKey aliased_pf):
       nh_(nh),
       frame_(frame),
       aliased_(aliased),
aliased_pf_(aliased_pf) {
    ;
    }
    public:
    NodeConstHandle get_aliased() const {
  NodeID id;
  if (nh_.get_has_value(aliased_)) {
   id= nh_.get_value(aliased_);
  } else {
   id= nh_.get_value(aliased_pf_, frame_);
  }
  return nh_.get_file().get_node_from_id(id);
}
    IMP_RMF_SHOWABLE(ConstStaticAlias,
                     "StaticAliasConst "
                     << nh_.get_name());
    ~StaticAliasConst() {
    }
    };

    typedef vector<StaticAliasConst>
            StaticAliasConsts;

/** Store a static reference to another node.

       \see StaticAliasConst
       \see StaticAliasFactory
     */
    class StaticAlias {
    NodeHandle nh_;
    int frame_;
    friend class StaticAliasFactory;
    private:
    internal::NodeIDLazyKey aliased_;
internal::NodeIDLazyKey aliased_pf_;
    StaticAlias(NodeHandle nh,
                       int frame,
                  internal::NodeIDLazyKey aliased,
internal::NodeIDLazyKey aliased_pf):
       nh_(nh),
       frame_(frame),
       aliased_(aliased),
aliased_pf_(aliased_pf) {
    ;
    }
    public:
    NodeHandle get_aliased() const {
  NodeID id;
  if (nh_.get_has_value(aliased_)) {
   id= nh_.get_value(aliased_);
  } else {
   id= nh_.get_value(aliased_pf_, frame_);
  }
  return nh_.get_file().get_node_from_id(id);
}
void set_aliased(NodeConstHandle v) {
  if (frame_ >=0) {
    nh_.set_value(aliased_pf_, v.get_id(), frame_);
  } else {
    return nh_.set_value(aliased_, v.get_id());
  }
}
    IMP_RMF_SHOWABLE(ConstStaticAlias,
                     "StaticAlias "
                     << nh_.get_name());
    ~StaticAlias() {
    }
    };

    typedef vector<StaticAlias>
            StaticAliass;

/** Create decorators of type StaticAlias.

       \see StaticAlias
       \see StaticAliasConstFactory
    */
    class StaticAliasFactory {
    private:
    internal::NodeIDLazyKey aliased_;
internal::NodeIDLazyKey aliased_pf_;
    public:
    typedef FileHandle File;
    typedef StaticAlias Decorator;
    StaticAliasFactory(FileHandle fh) {
    {
  CategoryD<1> cat=get_category_always<1>(fh, "alias");
aliased_=internal::NodeIDLazyKey(fh, cat,
                               "aliased",
                               false);
aliased_pf_=internal::NodeIDLazyKey(fh, cat,
                               "aliased",
                               true);
};
    }
    StaticAlias get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return StaticAlias(nh, frame, aliased_,
aliased_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(aliased_pf_, frame)
                          || nh.get_has_value(aliased_)))
                || ( frame <0 && (nh.get_has_value(aliased_)
                || nh.get_has_value(aliased_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(StaticAliasFactory,
                     "StaticAliasFactory");
    };

    typedef vector<StaticAliasFactory>
            StaticAliasFactories;

/** Create decorators of type StaticAlias.

       \see StaticAliasConst
       \see StaticAliasFactory
    */
    class StaticAliasConstFactory {
    private:
    NodeIDKey aliased_;
NodeIDKey aliased_pf_;
    public:
    typedef FileConstHandle File;
    typedef StaticAliasConst Decorator;
    StaticAliasConstFactory(FileConstHandle fh) {
    {
  CategoryD<1> cat=fh.get_category<1>("alias");
aliased_=(fh.get_has_key<NodeIDTraits>
                   (cat, "aliased", false)?
                   fh.get_key<NodeIDTraits>(cat,
                                     "aliased",
                                      false)
                              :NodeIDKey());
aliased_pf_=(fh.get_has_key<NodeIDTraits>
                   (cat, "aliased", true)?
                   fh.get_key<NodeIDTraits>(cat,
                                     "aliased",
                                      true)
                              :NodeIDKey());
};
    }
    StaticAliasConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return StaticAliasConst(nh, frame, aliased_,
aliased_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(aliased_pf_, frame)
                          || nh.get_has_value(aliased_)))
                || ( frame <0 && (nh.get_has_value(aliased_)
                || nh.get_has_value(aliased_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(StaticAliasConstFactory,
                     "StaticAliasConstFactory");
    };

    typedef vector<StaticAliasConstFactory>
            StaticAliasConstFactories;

/** Associate a score with some set of particles.

       \see Score
       \see ScoreConstFactory
     */
    class ScoreConst {
    NodeConstHandle nh_;
    int frame_;
    friend class ScoreConstFactory;
    private:
    StaticAliasConstFactory representation_;
FloatKey score_;
FloatKey score_pf_;
    ScoreConst(NodeConstHandle nh,
                       int frame,
                  StaticAliasConstFactory representation,
FloatKey score,
FloatKey score_pf):
       nh_(nh),
       frame_(frame),
       representation_(representation),
score_(score),
score_pf_(score_pf) {
    ;
    }
    public:
    NodeConstHandles get_representation() const {
  NodeConstHandles typed=nh_.get_children();
  NodeConstHandles ret;
  for (unsigned int i=0; i< typed.size(); ++i) {
     if (representation_.get_is(typed[i])) {
        ret.push_back(representation_.get(typed[i]).get_aliased());
     }
  }
  return ret;
}
Float get_score() const {
  if (nh_.get_has_value(score_)) {
   return nh_.get_value(score_);
  } else {
   return nh_.get_value(score_pf_, frame_);
  }
}
    IMP_RMF_SHOWABLE(ConstScore,
                     "ScoreConst "
                     << nh_.get_name());
    ~ScoreConst() {
    }
    };

    typedef vector<ScoreConst>
            ScoreConsts;

/** Associate a score with some set of particles.

       \see ScoreConst
       \see ScoreFactory
     */
    class Score {
    NodeHandle nh_;
    int frame_;
    friend class ScoreFactory;
    private:
    StaticAliasFactory representation_;
internal::FloatLazyKey score_;
internal::FloatLazyKey score_pf_;
    Score(NodeHandle nh,
                       int frame,
                  StaticAliasFactory representation,
internal::FloatLazyKey score,
internal::FloatLazyKey score_pf):
       nh_(nh),
       frame_(frame),
       representation_(representation),
score_(score),
score_pf_(score_pf) {
    ;
    }
    public:
    NodeHandles get_representation() const {
  NodeHandles typed=nh_.get_children();
  NodeHandles ret;
  for (unsigned int i=0; i< typed.size(); ++i) {
     if (representation_.get_is(typed[i])) {
        ret.push_back(representation_.get(typed[i]).get_aliased());
     }
  }
  return ret;
}
void set_representation(NodeConstHandles v) {
   for (unsigned int i=0; i< v.size(); ++i) {
       add_child_alias(nh_, v[i]);
   }
}
void set_representation(NodeHandles v) {
   for (unsigned int i=0; i< v.size(); ++i) {
       add_child_alias(nh_, v[i]);
   }
}
Float get_score() const {
  if (nh_.get_has_value(score_)) {
   return nh_.get_value(score_);
  } else {
   return nh_.get_value(score_pf_, frame_);
  }
}
void set_score(Float v) {
  if (frame_ >=0) {
    nh_.set_value(score_pf_, v, frame_);
  } else {
    return nh_.set_value(score_, v);
  }
}
    IMP_RMF_SHOWABLE(ConstScore,
                     "Score "
                     << nh_.get_name());
    ~Score() {
    }
    };

    typedef vector<Score>
            Scores;

/** Create decorators of type Score.

       \see Score
       \see ScoreConstFactory
    */
    class ScoreFactory {
    private:
    StaticAliasFactory representation_;
internal::FloatLazyKey score_;
internal::FloatLazyKey score_pf_;
    public:
    typedef FileHandle File;
    typedef Score Decorator;
    ScoreFactory(FileHandle fh) : representation_(fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "feature");
score_=internal::FloatLazyKey(fh, cat,
                               "score",
                               false);
score_pf_=internal::FloatLazyKey(fh, cat,
                               "score",
                               true);
};
    }
    Score get(NodeHandle nh,
                           int frame=-1) const {
      ;
      return Score(nh, frame, representation_,
score_,
score_pf_);
    }
    bool get_is(NodeHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(score_pf_, frame)
                          || nh.get_has_value(score_)))
                || ( frame <0 && (nh.get_has_value(score_)
                || nh.get_has_value(score_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ScoreFactory,
                     "ScoreFactory");
    };

    typedef vector<ScoreFactory>
            ScoreFactories;

/** Create decorators of type Score.

       \see ScoreConst
       \see ScoreFactory
    */
    class ScoreConstFactory {
    private:
    StaticAliasConstFactory representation_;
FloatKey score_;
FloatKey score_pf_;
    public:
    typedef FileConstHandle File;
    typedef ScoreConst Decorator;
    ScoreConstFactory(FileConstHandle fh) : representation_(fh){
    {
  CategoryD<1> cat=fh.get_category<1>("feature");
score_=(fh.get_has_key<FloatTraits>
                   (cat, "score", false)?
                   fh.get_key<FloatTraits>(cat,
                                     "score",
                                      false)
                              :FloatKey());
score_pf_=(fh.get_has_key<FloatTraits>
                   (cat, "score", true)?
                   fh.get_key<FloatTraits>(cat,
                                     "score",
                                      true)
                              :FloatKey());
};
    }
    ScoreConst get(NodeConstHandle nh,
                           int frame=-1) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ScoreConst(nh, frame, representation_,
score_,
score_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame=-1) const {
      return ((frame >=0 && (nh.get_has_value(score_pf_, frame)
                          || nh.get_has_value(score_)))
                || ( frame <0 && (nh.get_has_value(score_)
                || nh.get_has_value(score_pf_, 0))));
    }
    IMP_RMF_SHOWABLE(ScoreConstFactory,
                     "ScoreConstFactory");
    };

    typedef vector<ScoreConstFactory>
            ScoreConstFactories;

} /* namespace RMF */

#endif /* IMPLIBRMF_DECORATORS_H */
