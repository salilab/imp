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
namespace RMF {

/** These particles has associated color information.

       \see Colored
       \see ColoredConstFactory
     */
    class ColoredConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class ColoredConstFactory;
    private:
    FloatKeys rgb_color_;
    ColoredConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKeys rgb_color): nh_(nh),
                                      frame_(frame),
                                     rgb_color_(rgb_color) {
    ;
    }
    public:
    Floats get_rgb_color() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(rgb_color_[i], frame_));
       }
       return ret;
    }
    IMP_RMF_SHOWABLE(ConstColored,
                     "ColoredConst "
                     << nh_.get_name());
    ~ColoredConst() {
    }
    };

    typedef vector<ColoredConst> ColoredConsts;

/** These particles has associated color information.

       \see ColoredConst
       \see ColoredFactory
     */
    class Colored {
    NodeHandle nh_;
    unsigned int frame_;
    friend class ColoredFactory;
    private:
    FloatKeys rgb_color_;
    Colored(NodeHandle nh,
                      unsigned int frame,
                  FloatKeys rgb_color): nh_(nh),
                                      frame_(frame),
                                     rgb_color_(rgb_color) {
    ;
    }
    public:
    Floats get_rgb_color() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(rgb_color_[i], frame_));
       }
       return ret;
    }
void set_rgb_color(const Floats &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(rgb_color_[i], v[i], frame_);
         }
      }
    IMP_RMF_SHOWABLE(ConstColored,
                     "Colored "
                     << nh_.get_name());
    ~Colored() {
    }
    };

    typedef vector<Colored> Coloreds;

/** Create decorators of type Colored.

       \see Colored
       \see ColoredConstFactory
    */
    class ColoredFactory {
    private:
    FloatKeys rgb_color_;
    public:
    typedef FileHandle File;
    typedef Colored Decorator;
    ColoredFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
      rgb_color_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "rgb color red", false));
      rgb_color_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "rgb color green", false));
      rgb_color_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "rgb color blue", false));;
};
    }
    Colored get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Colored(nh, frame, rgb_color_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(rgb_color_[0], frame);
    }
    IMP_RMF_SHOWABLE(ColoredFactory, "ColoredFactory");
    };

    typedef vector<ColoredFactory> ColoredFactories;

/** Create decorators of type Colored.

       \see ColoredConst
       \see ColoredFactory
    */
    class ColoredConstFactory {
    private:
    FloatKeys rgb_color_;
    public:
    typedef FileConstHandle File;
    typedef ColoredConst Decorator;
    ColoredConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
      rgb_color_.push_back((fh.get_has_key<FloatTraits>(cat, "rgb color red")?
                   fh.get_key<FloatTraits>(cat, "rgb color red")
                              :FloatKey()));
      rgb_color_.push_back((fh.get_has_key<FloatTraits>(cat, "rgb color green")?
                   fh.get_key<FloatTraits>(cat, "rgb color green")
                              :FloatKey()));
      rgb_color_.push_back((fh.get_has_key<FloatTraits>(cat, "rgb color blue")?
                   fh.get_key<FloatTraits>(cat, "rgb color blue")
                              :FloatKey()));;
};
    }
    ColoredConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ColoredConst(nh, frame, rgb_color_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(rgb_color_[0], frame);
    }
    IMP_RMF_SHOWABLE(ColoredConstFactory, "ColoredConstFactory");
    };

    typedef vector<ColoredConstFactory> ColoredConstFactories;

/** These particles has associated coordinates and radius information.

       \see Particle
       \see ParticleConstFactory
     */
    class ParticleConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class ParticleConstFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    ParticleConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
mass_(mass) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
Float get_mass() const {
  return nh_.get_value(mass_, frame_);
}
    IMP_RMF_SHOWABLE(ConstParticle,
                     "ParticleConst "
                     << nh_.get_name());
    ~ParticleConst() {
    }
    };

    typedef vector<ParticleConst> ParticleConsts;

/** These particles has associated coordinates and radius information.

       \see ParticleConst
       \see ParticleFactory
     */
    class Particle {
    NodeHandle nh_;
    unsigned int frame_;
    friend class ParticleFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    Particle(NodeHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
mass_(mass) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const Floats &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
void set_radius(Float v) {
   nh_.set_value(radius_, v, frame_);
}
Float get_mass() const {
  return nh_.get_value(mass_, frame_);
}
void set_mass(Float v) {
   nh_.set_value(mass_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstParticle,
                     "Particle "
                     << nh_.get_name());
    ~Particle() {
    }
    };

    typedef vector<Particle> Particles;

/** Create decorators of type Particle.

       \see Particle
       \see ParticleConstFactory
    */
    class ParticleFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    public:
    typedef FileHandle File;
    typedef Particle Decorator;
    ParticleFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian x", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian y", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian z", true));;
radius_=get_key_always<FloatTraits>(fh, cat,
                               "radius", false);
mass_=get_key_always<FloatTraits>(fh, cat,
                               "mass", false);
};
    }
    Particle get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Particle(nh, frame, coordinates_,
radius_,
mass_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(mass_, frame);
    }
    IMP_RMF_SHOWABLE(ParticleFactory, "ParticleFactory");
    };

    typedef vector<ParticleFactory> ParticleFactories;

/** Create decorators of type Particle.

       \see ParticleConst
       \see ParticleFactory
    */
    class ParticleConstFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    public:
    typedef FileConstHandle File;
    typedef ParticleConst Decorator;
    ParticleConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
                   fh.get_key<FloatTraits>(cat, "cartesian x")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
                   fh.get_key<FloatTraits>(cat, "cartesian y")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
                   fh.get_key<FloatTraits>(cat, "cartesian z")
                              :FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
                   fh.get_key<FloatTraits>(cat, "radius")
                              :FloatKey());
mass_=(fh.get_has_key<FloatTraits>(cat, "mass")?
                   fh.get_key<FloatTraits>(cat, "mass")
                              :FloatKey());
};
    }
    ParticleConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ParticleConst(nh, frame, coordinates_,
radius_,
mass_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(mass_, frame);
    }
    IMP_RMF_SHOWABLE(ParticleConstFactory, "ParticleConstFactory");
    };

    typedef vector<ParticleConstFactory> ParticleConstFactories;

/** These particles has associated coordinates and radius information.

       \see IntermediateParticle
       \see IntermediateParticleConstFactory
     */
    class IntermediateParticleConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class IntermediateParticleConstFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
    IntermediateParticleConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
    IMP_RMF_SHOWABLE(ConstIntermediateParticle,
                     "IntermediateParticleConst "
                     << nh_.get_name());
    ~IntermediateParticleConst() {
    }
    };

    typedef vector<IntermediateParticleConst> IntermediateParticleConsts;

/** These particles has associated coordinates and radius information.

       \see IntermediateParticleConst
       \see IntermediateParticleFactory
     */
    class IntermediateParticle {
    NodeHandle nh_;
    unsigned int frame_;
    friend class IntermediateParticleFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
    IntermediateParticle(NodeHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const Floats &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
void set_radius(Float v) {
   nh_.set_value(radius_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstIntermediateParticle,
                     "IntermediateParticle "
                     << nh_.get_name());
    ~IntermediateParticle() {
    }
    };

    typedef vector<IntermediateParticle> IntermediateParticles;

/** Create decorators of type IntermediateParticle.

       \see IntermediateParticle
       \see IntermediateParticleConstFactory
    */
    class IntermediateParticleFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
    public:
    typedef FileHandle File;
    typedef IntermediateParticle Decorator;
    IntermediateParticleFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian x", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian y", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian z", true));;
radius_=get_key_always<FloatTraits>(fh, cat,
                               "radius", false);
};
    }
    IntermediateParticle get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return IntermediateParticle(nh, frame, coordinates_,
radius_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame);
    }
    IMP_RMF_SHOWABLE(IntermediateParticleFactory,
                     "IntermediateParticleFactory");
    };

    typedef vector<IntermediateParticleFactory> IntermediateParticleFactories;

/** Create decorators of type IntermediateParticle.

       \see IntermediateParticleConst
       \see IntermediateParticleFactory
    */
    class IntermediateParticleConstFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
    public:
    typedef FileConstHandle File;
    typedef IntermediateParticleConst Decorator;
    IntermediateParticleConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
                   fh.get_key<FloatTraits>(cat, "cartesian x")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
                   fh.get_key<FloatTraits>(cat, "cartesian y")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
                   fh.get_key<FloatTraits>(cat, "cartesian z")
                              :FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
                   fh.get_key<FloatTraits>(cat, "radius")
                              :FloatKey());
};
    }
    IntermediateParticleConst get(NodeConstHandle nh, unsigned int frame=0)
      const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return IntermediateParticleConst(nh, frame, coordinates_,
radius_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame);
    }
    IMP_RMF_SHOWABLE(IntermediateParticleConstFactory,
                     "IntermediateParticleConstFactory");
    };

    typedef vector<IntermediateParticleConstFactory>
    IntermediateParticleConstFactories;

/** These particles has associated coordinates and radius information.

       \see RigidParticle
       \see RigidParticleConstFactory
     */
    class RigidParticleConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class RigidParticleConstFactory;
    private:
    FloatKeys orientation_;
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    RigidParticleConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKeys orientation,
FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),
                                      frame_(frame),
                                     orientation_(orientation),
coordinates_(coordinates),
radius_(radius),
mass_(mass) {
    ;
    }
    public:
    Floats get_orientation() const {
       Floats ret;
       for (unsigned int i=0; i< 4; ++i) {
          ret.push_back(nh_.get_value(orientation_[i], frame_));
       }
       return ret;
    }
Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
Float get_mass() const {
  return nh_.get_value(mass_, frame_);
}
    IMP_RMF_SHOWABLE(ConstRigidParticle,
                     "RigidParticleConst "
                     << nh_.get_name());
    ~RigidParticleConst() {
    }
    };

    typedef vector<RigidParticleConst> RigidParticleConsts;

/** These particles has associated coordinates and radius information.

       \see RigidParticleConst
       \see RigidParticleFactory
     */
    class RigidParticle {
    NodeHandle nh_;
    unsigned int frame_;
    friend class RigidParticleFactory;
    private:
    FloatKeys orientation_;
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    RigidParticle(NodeHandle nh,
                      unsigned int frame,
                  FloatKeys orientation,
FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),
                                      frame_(frame),
                                     orientation_(orientation),
coordinates_(coordinates),
radius_(radius),
mass_(mass) {
    ;
    }
    public:
    Floats get_orientation() const {
       Floats ret;
       for (unsigned int i=0; i< 4; ++i) {
          ret.push_back(nh_.get_value(orientation_[i], frame_));
       }
       return ret;
    }
void set_orientation(const Floats &v) {
         for (unsigned int i=0; i< 4; ++i) {
            nh_.set_value(orientation_[i], v[i], frame_);
         }
      }
Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const Floats &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
void set_radius(Float v) {
   nh_.set_value(radius_, v, frame_);
}
Float get_mass() const {
  return nh_.get_value(mass_, frame_);
}
void set_mass(Float v) {
   nh_.set_value(mass_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstRigidParticle,
                     "RigidParticle "
                     << nh_.get_name());
    ~RigidParticle() {
    }
    };

    typedef vector<RigidParticle> RigidParticles;

/** Create decorators of type RigidParticle.

       \see RigidParticle
       \see RigidParticleConstFactory
    */
    class RigidParticleFactory {
    private:
    FloatKeys orientation_;
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    public:
    typedef FileHandle File;
    typedef RigidParticle Decorator;
    RigidParticleFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
      orientation_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "orientation r", true));
      orientation_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "orientation i", true));
      orientation_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "orientation j", true));
      orientation_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "orientation k", true));;
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian x", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian y", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian z", true));;
radius_=get_key_always<FloatTraits>(fh, cat,
                               "radius", false);
mass_=get_key_always<FloatTraits>(fh, cat,
                               "mass", false);
};
    }
    RigidParticle get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return RigidParticle(nh, frame, orientation_,
coordinates_,
radius_,
mass_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(orientation_[0], frame)
    && nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(mass_, frame);
    }
    IMP_RMF_SHOWABLE(RigidParticleFactory, "RigidParticleFactory");
    };

    typedef vector<RigidParticleFactory> RigidParticleFactories;

/** Create decorators of type RigidParticle.

       \see RigidParticleConst
       \see RigidParticleFactory
    */
    class RigidParticleConstFactory {
    private:
    FloatKeys orientation_;
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
    public:
    typedef FileConstHandle File;
    typedef RigidParticleConst Decorator;
    RigidParticleConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation r")?
                   fh.get_key<FloatTraits>(cat, "orientation r")
                              :FloatKey()));
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation i")?
                   fh.get_key<FloatTraits>(cat, "orientation i")
                              :FloatKey()));
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation j")?
                   fh.get_key<FloatTraits>(cat, "orientation j")
                              :FloatKey()));
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation k")?
                   fh.get_key<FloatTraits>(cat, "orientation k")
                              :FloatKey()));;
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
                   fh.get_key<FloatTraits>(cat, "cartesian x")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
                   fh.get_key<FloatTraits>(cat, "cartesian y")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
                   fh.get_key<FloatTraits>(cat, "cartesian z")
                              :FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
                   fh.get_key<FloatTraits>(cat, "radius")
                              :FloatKey());
mass_=(fh.get_has_key<FloatTraits>(cat, "mass")?
                   fh.get_key<FloatTraits>(cat, "mass")
                              :FloatKey());
};
    }
    RigidParticleConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return RigidParticleConst(nh, frame, orientation_,
coordinates_,
radius_,
mass_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(orientation_[0], frame)
    && nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(mass_, frame);
    }
    IMP_RMF_SHOWABLE(RigidParticleConstFactory, "RigidParticleConstFactory");
    };

    typedef vector<RigidParticleConstFactory> RigidParticleConstFactories;

/** Associate a score with some set of particles.

       \see Score
       \see ScoreConstFactory
     */
    class ScoreConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class ScoreConstFactory;
    private:
    NodeIDsKey representation_;
FloatKey score_;
    ScoreConst(NodeConstHandle nh,
                      unsigned int frame,
                  NodeIDsKey representation,
FloatKey score): nh_(nh),
                                      frame_(frame),
                                     representation_(representation),
score_(score) {
    ;
    }
    public:
    NodeIDs get_representation() const {
  return nh_.get_value(representation_, frame_);
}
Float get_score() const {
  return nh_.get_value(score_, frame_);
}
    IMP_RMF_SHOWABLE(ConstScore,
                     "ScoreConst "
                     << nh_.get_name());
    ~ScoreConst() {
    }
    };

    typedef vector<ScoreConst> ScoreConsts;

/** Associate a score with some set of particles.

       \see ScoreConst
       \see ScoreFactory
     */
    class Score {
    NodeHandle nh_;
    unsigned int frame_;
    friend class ScoreFactory;
    private:
    NodeIDsKey representation_;
FloatKey score_;
    Score(NodeHandle nh,
                      unsigned int frame,
                  NodeIDsKey representation,
FloatKey score): nh_(nh),
                                      frame_(frame),
                                     representation_(representation),
score_(score) {
    ;
    }
    public:
    NodeIDs get_representation() const {
  return nh_.get_value(representation_, frame_);
}
void set_representation(NodeIDs v) {
   nh_.set_value(representation_, v, frame_);
}
Float get_score() const {
  return nh_.get_value(score_, frame_);
}
void set_score(Float v) {
   nh_.set_value(score_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstScore,
                     "Score "
                     << nh_.get_name());
    ~Score() {
    }
    };

    typedef vector<Score> Scores;

/** Create decorators of type Score.

       \see Score
       \see ScoreConstFactory
    */
    class ScoreFactory {
    private:
    NodeIDsKey representation_;
FloatKey score_;
    public:
    typedef FileHandle File;
    typedef Score Decorator;
    ScoreFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "feature");
representation_=get_key_always<NodeIDsTraits>(fh, cat,
                               "representation", false);
score_=get_key_always<FloatTraits>(fh, cat,
                               "score", false);
};
    }
    Score get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Score(nh, frame, representation_,
score_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(representation_, frame)
    && nh.get_has_value(score_, frame);
    }
    IMP_RMF_SHOWABLE(ScoreFactory, "ScoreFactory");
    };

    typedef vector<ScoreFactory> ScoreFactories;

/** Create decorators of type Score.

       \see ScoreConst
       \see ScoreFactory
    */
    class ScoreConstFactory {
    private:
    NodeIDsKey representation_;
FloatKey score_;
    public:
    typedef FileConstHandle File;
    typedef ScoreConst Decorator;
    ScoreConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("feature");
representation_=(fh.get_has_key<NodeIDsTraits>(cat, "representation")?
                   fh.get_key<NodeIDsTraits>(cat, "representation")
                              :NodeIDsKey());
score_=(fh.get_has_key<FloatTraits>(cat, "score")?
                   fh.get_key<FloatTraits>(cat, "score")
                              :FloatKey());
};
    }
    ScoreConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ScoreConst(nh, frame, representation_,
score_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(representation_, frame)
    && nh.get_has_value(score_, frame);
    }
    IMP_RMF_SHOWABLE(ScoreConstFactory, "ScoreConstFactory");
    };

    typedef vector<ScoreConstFactory> ScoreConstFactories;

/** A geometric ball.

       \see Ball
       \see BallConstFactory
     */
    class BallConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class BallConstFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    BallConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius,
IndexKey type): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
type_(type) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
    IMP_RMF_SHOWABLE(ConstBall,
                     "BallConst "
                     << nh_.get_name());
    ~BallConst() {
    }
    };

    typedef vector<BallConst> BallConsts;

/** A geometric ball.

       \see BallConst
       \see BallFactory
     */
    class Ball {
    NodeHandle nh_;
    unsigned int frame_;
    friend class BallFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    Ball(NodeHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius,
IndexKey type): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
type_(type) {
    nh.set_value(type_, 0);;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const Floats &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
void set_radius(Float v) {
   nh_.set_value(radius_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstBall,
                     "Ball "
                     << nh_.get_name());
    ~Ball() {
    }
    };

    typedef vector<Ball> Balls;

/** Create decorators of type Ball.

       \see Ball
       \see BallConstFactory
    */
    class BallFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    public:
    typedef FileHandle File;
    typedef Ball Decorator;
    BallFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian x", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian y", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian z", true));;
radius_=get_key_always<FloatTraits>(fh, cat,
                               "radius", false);
type_=get_key_always<IndexTraits>(fh, cat,
                               "type", false);
};
    }
    Ball get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Ball(nh, frame, coordinates_,
radius_,
type_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(type_, frame)
    && nh.get_value(type_)==0;
    }
    IMP_RMF_SHOWABLE(BallFactory, "BallFactory");
    };

    typedef vector<BallFactory> BallFactories;

/** Create decorators of type Ball.

       \see BallConst
       \see BallFactory
    */
    class BallConstFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    public:
    typedef FileConstHandle File;
    typedef BallConst Decorator;
    BallConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
                   fh.get_key<FloatTraits>(cat, "cartesian x")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
                   fh.get_key<FloatTraits>(cat, "cartesian y")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
                   fh.get_key<FloatTraits>(cat, "cartesian z")
                              :FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
                   fh.get_key<FloatTraits>(cat, "radius")
                              :FloatKey());
type_=(fh.get_has_key<IndexTraits>(cat, "type")?
                   fh.get_key<IndexTraits>(cat, "type")
                              :IndexKey());
};
    }
    BallConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return BallConst(nh, frame, coordinates_,
radius_,
type_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(type_, frame)
    && nh.get_value(type_)==0;
    }
    IMP_RMF_SHOWABLE(BallConstFactory, "BallConstFactory");
    };

    typedef vector<BallConstFactory> BallConstFactories;

/** A geometric cylinder.

       \see Cylinder
       \see CylinderConstFactory
     */
    class CylinderConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class CylinderConstFactory;
    private:
    FloatsKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    CylinderConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatsKeys coordinates,
FloatKey radius,
IndexKey type): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
type_(type) {
    ;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
    IMP_RMF_SHOWABLE(ConstCylinder,
                     "CylinderConst "
                     << nh_.get_name());
    ~CylinderConst() {
    }
    };

    typedef vector<CylinderConst> CylinderConsts;

/** A geometric cylinder.

       \see CylinderConst
       \see CylinderFactory
     */
    class Cylinder {
    NodeHandle nh_;
    unsigned int frame_;
    friend class CylinderFactory;
    private:
    FloatsKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    Cylinder(NodeHandle nh,
                      unsigned int frame,
                  FloatsKeys coordinates,
FloatKey radius,
IndexKey type): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
type_(type) {
    nh.set_value(type_, 1);;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const FloatsList &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
void set_radius(Float v) {
   nh_.set_value(radius_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstCylinder,
                     "Cylinder "
                     << nh_.get_name());
    ~Cylinder() {
    }
    };

    typedef vector<Cylinder> Cylinders;

/** Create decorators of type Cylinder.

       \see Cylinder
       \see CylinderConstFactory
    */
    class CylinderFactory {
    private:
    FloatsKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    public:
    typedef FileHandle File;
    typedef Cylinder Decorator;
    CylinderFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                               "cartesian xs", true));
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                               "cartesian ys", true));
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                               "cartesian zs", true));;
radius_=get_key_always<FloatTraits>(fh, cat,
                               "radius", false);
type_=get_key_always<IndexTraits>(fh, cat,
                               "type", false);
};
    }
    Cylinder get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Cylinder(nh, frame, coordinates_,
radius_,
type_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(type_, frame)
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(CylinderFactory, "CylinderFactory");
    };

    typedef vector<CylinderFactory> CylinderFactories;

/** Create decorators of type Cylinder.

       \see CylinderConst
       \see CylinderFactory
    */
    class CylinderConstFactory {
    private:
    FloatsKeys coordinates_;
FloatKey radius_;
IndexKey type_;
    public:
    typedef FileConstHandle File;
    typedef CylinderConst Decorator;
    CylinderConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian xs")?
                   fh.get_key<FloatsTraits>(cat, "cartesian xs")
                              :FloatsKey()));
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian ys")?
                   fh.get_key<FloatsTraits>(cat, "cartesian ys")
                              :FloatsKey()));
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian zs")?
                   fh.get_key<FloatsTraits>(cat, "cartesian zs")
                              :FloatsKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
                   fh.get_key<FloatTraits>(cat, "radius")
                              :FloatKey());
type_=(fh.get_has_key<IndexTraits>(cat, "type")?
                   fh.get_key<IndexTraits>(cat, "type")
                              :IndexKey());
};
    }
    CylinderConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return CylinderConst(nh, frame, coordinates_,
radius_,
type_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(type_, frame)
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(CylinderConstFactory, "CylinderConstFactory");
    };

    typedef vector<CylinderConstFactory> CylinderConstFactories;

/** A geometric line setgment.

       \see Segment
       \see SegmentConstFactory
     */
    class SegmentConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class SegmentConstFactory;
    private:
    FloatsKeys coordinates_;
IndexKey type_;
    SegmentConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatsKeys coordinates,
IndexKey type): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
type_(type) {
    ;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
    IMP_RMF_SHOWABLE(ConstSegment,
                     "SegmentConst "
                     << nh_.get_name());
    ~SegmentConst() {
    }
    };

    typedef vector<SegmentConst> SegmentConsts;

/** A geometric line setgment.

       \see SegmentConst
       \see SegmentFactory
     */
    class Segment {
    NodeHandle nh_;
    unsigned int frame_;
    friend class SegmentFactory;
    private:
    FloatsKeys coordinates_;
IndexKey type_;
    Segment(NodeHandle nh,
                      unsigned int frame,
                  FloatsKeys coordinates,
IndexKey type): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
type_(type) {
    nh.set_value(type_, 1);;
    }
    public:
    FloatsList get_coordinates() const {
       FloatsList ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const FloatsList &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
    IMP_RMF_SHOWABLE(ConstSegment,
                     "Segment "
                     << nh_.get_name());
    ~Segment() {
    }
    };

    typedef vector<Segment> Segments;

/** Create decorators of type Segment.

       \see Segment
       \see SegmentConstFactory
    */
    class SegmentFactory {
    private:
    FloatsKeys coordinates_;
IndexKey type_;
    public:
    typedef FileHandle File;
    typedef Segment Decorator;
    SegmentFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                               "cartesian xs", true));
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                               "cartesian ys", true));
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                               "cartesian zs", true));;
type_=get_key_always<IndexTraits>(fh, cat,
                               "type", false);
};
    }
    Segment get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Segment(nh, frame, coordinates_,
type_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(type_, frame)
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(SegmentFactory, "SegmentFactory");
    };

    typedef vector<SegmentFactory> SegmentFactories;

/** Create decorators of type Segment.

       \see SegmentConst
       \see SegmentFactory
    */
    class SegmentConstFactory {
    private:
    FloatsKeys coordinates_;
IndexKey type_;
    public:
    typedef FileConstHandle File;
    typedef SegmentConst Decorator;
    SegmentConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("shape");
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian xs")?
                   fh.get_key<FloatsTraits>(cat, "cartesian xs")
                              :FloatsKey()));
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian ys")?
                   fh.get_key<FloatsTraits>(cat, "cartesian ys")
                              :FloatsKey()));
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian zs")?
                   fh.get_key<FloatsTraits>(cat, "cartesian zs")
                              :FloatsKey()));;
type_=(fh.get_has_key<IndexTraits>(cat, "type")?
                   fh.get_key<IndexTraits>(cat, "type")
                              :IndexKey());
};
    }
    SegmentConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return SegmentConst(nh, frame, coordinates_,
type_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(type_, frame)
    && nh.get_value(type_)==1;
    }
    IMP_RMF_SHOWABLE(SegmentConstFactory, "SegmentConstFactory");
    };

    typedef vector<SegmentConstFactory> SegmentConstFactories;

/** Information regarding a publication.

       \see JournalArticle
       \see JournalArticleConstFactory
     */
    class JournalArticleConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class JournalArticleConstFactory;
    private:
    StringKey title_;
StringKey journal_;
StringKey pubmed_id_;
IntKey year_;
StringsKey authors_;
    JournalArticleConst(NodeConstHandle nh,
                      unsigned int frame,
                  StringKey title,
StringKey journal,
StringKey pubmed_id,
IntKey year,
StringsKey authors): nh_(nh),
                                      frame_(frame),
                                     title_(title),
journal_(journal),
pubmed_id_(pubmed_id),
year_(year),
authors_(authors) {
    ;
    }
    public:
    String get_title() const {
  return nh_.get_value(title_, frame_);
}
String get_journal() const {
  return nh_.get_value(journal_, frame_);
}
String get_pubmed_id() const {
  return nh_.get_value(pubmed_id_, frame_);
}
Int get_year() const {
  return nh_.get_value(year_, frame_);
}
Strings get_authors() const {
  return nh_.get_value(authors_, frame_);
}
    IMP_RMF_SHOWABLE(ConstJournalArticle,
                     "JournalArticleConst "
                     << nh_.get_name());
    ~JournalArticleConst() {
    }
    };

    typedef vector<JournalArticleConst> JournalArticleConsts;

/** Information regarding a publication.

       \see JournalArticleConst
       \see JournalArticleFactory
     */
    class JournalArticle {
    NodeHandle nh_;
    unsigned int frame_;
    friend class JournalArticleFactory;
    private:
    StringKey title_;
StringKey journal_;
StringKey pubmed_id_;
IntKey year_;
StringsKey authors_;
    JournalArticle(NodeHandle nh,
                      unsigned int frame,
                  StringKey title,
StringKey journal,
StringKey pubmed_id,
IntKey year,
StringsKey authors): nh_(nh),
                                      frame_(frame),
                                     title_(title),
journal_(journal),
pubmed_id_(pubmed_id),
year_(year),
authors_(authors) {
    ;
    }
    public:
    String get_title() const {
  return nh_.get_value(title_, frame_);
}
void set_title(String v) {
   nh_.set_value(title_, v, frame_);
}
String get_journal() const {
  return nh_.get_value(journal_, frame_);
}
void set_journal(String v) {
   nh_.set_value(journal_, v, frame_);
}
String get_pubmed_id() const {
  return nh_.get_value(pubmed_id_, frame_);
}
void set_pubmed_id(String v) {
   nh_.set_value(pubmed_id_, v, frame_);
}
Int get_year() const {
  return nh_.get_value(year_, frame_);
}
void set_year(Int v) {
   nh_.set_value(year_, v, frame_);
}
Strings get_authors() const {
  return nh_.get_value(authors_, frame_);
}
void set_authors(Strings v) {
   nh_.set_value(authors_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstJournalArticle,
                     "JournalArticle "
                     << nh_.get_name());
    ~JournalArticle() {
    }
    };

    typedef vector<JournalArticle> JournalArticles;

/** Create decorators of type JournalArticle.

       \see JournalArticle
       \see JournalArticleConstFactory
    */
    class JournalArticleFactory {
    private:
    StringKey title_;
StringKey journal_;
StringKey pubmed_id_;
IntKey year_;
StringsKey authors_;
    public:
    typedef FileHandle File;
    typedef JournalArticle Decorator;
    JournalArticleFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "publication");
title_=get_key_always<StringTraits>(fh, cat,
                               "title", false);
journal_=get_key_always<StringTraits>(fh, cat,
                               "journal", false);
pubmed_id_=get_key_always<StringTraits>(fh, cat,
                               "pubmed id", false);
year_=get_key_always<IntTraits>(fh, cat,
                               "year", false);
authors_=get_key_always<StringsTraits>(fh, cat,
                               "authors", false);
};
    }
    JournalArticle get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return JournalArticle(nh, frame, title_,
journal_,
pubmed_id_,
year_,
authors_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(title_, frame)
    && nh.get_has_value(journal_, frame)
    && nh.get_has_value(pubmed_id_, frame)
    && nh.get_has_value(year_, frame)
    && nh.get_has_value(authors_, frame);
    }
    IMP_RMF_SHOWABLE(JournalArticleFactory, "JournalArticleFactory");
    };

    typedef vector<JournalArticleFactory> JournalArticleFactories;

/** Create decorators of type JournalArticle.

       \see JournalArticleConst
       \see JournalArticleFactory
    */
    class JournalArticleConstFactory {
    private:
    StringKey title_;
StringKey journal_;
StringKey pubmed_id_;
IntKey year_;
StringsKey authors_;
    public:
    typedef FileConstHandle File;
    typedef JournalArticleConst Decorator;
    JournalArticleConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("publication");
title_=(fh.get_has_key<StringTraits>(cat, "title")?
                   fh.get_key<StringTraits>(cat, "title")
                              :StringKey());
journal_=(fh.get_has_key<StringTraits>(cat, "journal")?
                   fh.get_key<StringTraits>(cat, "journal")
                              :StringKey());
pubmed_id_=(fh.get_has_key<StringTraits>(cat, "pubmed id")?
                   fh.get_key<StringTraits>(cat, "pubmed id")
                              :StringKey());
year_=(fh.get_has_key<IntTraits>(cat, "year")?
                   fh.get_key<IntTraits>(cat, "year")
                              :IntKey());
authors_=(fh.get_has_key<StringsTraits>(cat, "authors")?
                   fh.get_key<StringsTraits>(cat, "authors")
                              :StringsKey());
};
    }
    JournalArticleConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return JournalArticleConst(nh, frame, title_,
journal_,
pubmed_id_,
year_,
authors_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(title_, frame)
    && nh.get_has_value(journal_, frame)
    && nh.get_has_value(pubmed_id_, frame)
    && nh.get_has_value(year_, frame)
    && nh.get_has_value(authors_, frame);
    }
    IMP_RMF_SHOWABLE(JournalArticleConstFactory, "JournalArticleConstFactory");
    };

    typedef vector<JournalArticleConstFactory> JournalArticleConstFactories;

/** Information regarding a residue.

       \see Residue
       \see ResidueConstFactory
     */
    class ResidueConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class ResidueConstFactory;
    private:
    boost::array<IntKey,2> index_;
StringKey type_;
    ResidueConst(NodeConstHandle nh,
                      unsigned int frame,
                  boost::array<IntKey, 2> index,
StringKey type): nh_(nh),
                                      frame_(frame),
                                     index_(index),
type_(type) {
    ;
    }
    public:
    Int get_index() const {
  return nh_.get_value(index_[0], frame_);
}
String get_type() const {
  return nh_.get_value(type_, frame_);
}
    IMP_RMF_SHOWABLE(ConstResidue,
                     "ResidueConst "
                     << nh_.get_name());
    ~ResidueConst() {
    }
    };

    typedef vector<ResidueConst> ResidueConsts;

/** Information regarding a residue.

       \see ResidueConst
       \see ResidueFactory
     */
    class Residue {
    NodeHandle nh_;
    unsigned int frame_;
    friend class ResidueFactory;
    private:
    boost::array<IntKey,2> index_;
StringKey type_;
    Residue(NodeHandle nh,
                      unsigned int frame,
                  boost::array<IntKey, 2> index,
StringKey type): nh_(nh),
                                      frame_(frame),
                                     index_(index),
type_(type) {
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
  return nh_.get_value(type_, frame_);
}
void set_type(String v) {
   nh_.set_value(type_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstResidue,
                     "Residue "
                     << nh_.get_name());
    ~Residue() {
    }
    };

    typedef vector<Residue> Residues;

/** Create decorators of type Residue.

       \see Residue
       \see ResidueConstFactory
    */
    class ResidueFactory {
    private:
    boost::array<IntKey,2> index_;
StringKey type_;
    public:
    typedef FileHandle File;
    typedef Residue Decorator;
    ResidueFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
index_[0]=get_key_always<IntTraits>(fh, cat,
                               "first residue index", false);
index_[1]=get_key_always<IntTraits>(fh, cat,
                               "last residue index", false);
type_=get_key_always<StringTraits>(fh, cat,
                               "residue type", false);
};
    }
    Residue get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Residue(nh, frame, index_,
type_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(index_[0], frame)
  && nh.get_has_value(index_[1], frame)
  && nh.get_value(index_[0], frame)
   ==nh.get_value(index_[1], frame)
    && nh.get_has_value(type_, frame);
    }
    IMP_RMF_SHOWABLE(ResidueFactory, "ResidueFactory");
    };

    typedef vector<ResidueFactory> ResidueFactories;

/** Create decorators of type Residue.

       \see ResidueConst
       \see ResidueFactory
    */
    class ResidueConstFactory {
    private:
    boost::array<IntKey,2> index_;
StringKey type_;
    public:
    typedef FileConstHandle File;
    typedef ResidueConst Decorator;
    ResidueConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
index_[0]=(fh.get_has_key<IntTraits>(cat, "first residue index")?
                   fh.get_key<IntTraits>(cat, "first residue index")
                              :IntKey());
index_[1]=(fh.get_has_key<IntTraits>(cat, "last residue index")?
                   fh.get_key<IntTraits>(cat, "last residue index")
                              :IntKey());
type_=(fh.get_has_key<StringTraits>(cat, "residue type")?
                   fh.get_key<StringTraits>(cat, "residue type")
                              :StringKey());
};
    }
    ResidueConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ResidueConst(nh, frame, index_,
type_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(index_[0], frame)
  && nh.get_has_value(index_[1], frame)
  && nh.get_value(index_[0], frame)
   ==nh.get_value(index_[1], frame)
    && nh.get_has_value(type_, frame);
    }
    IMP_RMF_SHOWABLE(ResidueConstFactory, "ResidueConstFactory");
    };

    typedef vector<ResidueConstFactory> ResidueConstFactories;

/** Information regarding an atom.

       \see Atom
       \see AtomConstFactory
     */
    class AtomConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class AtomConstFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
IndexKey element_;
    AtomConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius,
FloatKey mass,
IndexKey element): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
mass_(mass),
element_(element) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
Float get_mass() const {
  return nh_.get_value(mass_, frame_);
}
Index get_element() const {
  return nh_.get_value(element_, frame_);
}
    IMP_RMF_SHOWABLE(ConstAtom,
                     "AtomConst "
                     << nh_.get_name());
    ~AtomConst() {
    }
    };

    typedef vector<AtomConst> AtomConsts;

/** Information regarding an atom.

       \see AtomConst
       \see AtomFactory
     */
    class Atom {
    NodeHandle nh_;
    unsigned int frame_;
    friend class AtomFactory;
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
IndexKey element_;
    Atom(NodeHandle nh,
                      unsigned int frame,
                  FloatKeys coordinates,
FloatKey radius,
FloatKey mass,
IndexKey element): nh_(nh),
                                      frame_(frame),
                                     coordinates_(coordinates),
radius_(radius),
mass_(mass),
element_(element) {
    ;
    }
    public:
    Floats get_coordinates() const {
       Floats ret;
       for (unsigned int i=0; i< 3; ++i) {
          ret.push_back(nh_.get_value(coordinates_[i], frame_));
       }
       return ret;
    }
void set_coordinates(const Floats &v) {
         for (unsigned int i=0; i< 3; ++i) {
            nh_.set_value(coordinates_[i], v[i], frame_);
         }
      }
Float get_radius() const {
  return nh_.get_value(radius_, frame_);
}
void set_radius(Float v) {
   nh_.set_value(radius_, v, frame_);
}
Float get_mass() const {
  return nh_.get_value(mass_, frame_);
}
void set_mass(Float v) {
   nh_.set_value(mass_, v, frame_);
}
Index get_element() const {
  return nh_.get_value(element_, frame_);
}
void set_element(Index v) {
   nh_.set_value(element_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstAtom,
                     "Atom "
                     << nh_.get_name());
    ~Atom() {
    }
    };

    typedef vector<Atom> Atoms;

/** Create decorators of type Atom.

       \see Atom
       \see AtomConstFactory
    */
    class AtomFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
IndexKey element_;
    public:
    typedef FileHandle File;
    typedef Atom Decorator;
    AtomFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian x", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian y", true));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                               "cartesian z", true));;
radius_=get_key_always<FloatTraits>(fh, cat,
                               "radius", false);
mass_=get_key_always<FloatTraits>(fh, cat,
                               "mass", false);
element_=get_key_always<IndexTraits>(fh, cat,
                               "element", false);
};
    }
    Atom get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Atom(nh, frame, coordinates_,
radius_,
mass_,
element_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(mass_, frame)
    && nh.get_has_value(element_, frame);
    }
    IMP_RMF_SHOWABLE(AtomFactory, "AtomFactory");
    };

    typedef vector<AtomFactory> AtomFactories;

/** Create decorators of type Atom.

       \see AtomConst
       \see AtomFactory
    */
    class AtomConstFactory {
    private:
    FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
IndexKey element_;
    public:
    typedef FileConstHandle File;
    typedef AtomConst Decorator;
    AtomConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
                   fh.get_key<FloatTraits>(cat, "cartesian x")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
                   fh.get_key<FloatTraits>(cat, "cartesian y")
                              :FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
                   fh.get_key<FloatTraits>(cat, "cartesian z")
                              :FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
                   fh.get_key<FloatTraits>(cat, "radius")
                              :FloatKey());
mass_=(fh.get_has_key<FloatTraits>(cat, "mass")?
                   fh.get_key<FloatTraits>(cat, "mass")
                              :FloatKey());
element_=(fh.get_has_key<IndexTraits>(cat, "element")?
                   fh.get_key<IndexTraits>(cat, "element")
                              :IndexKey());
};
    }
    AtomConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return AtomConst(nh, frame, coordinates_,
radius_,
mass_,
element_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(coordinates_[0], frame)
    && nh.get_has_value(radius_, frame)
    && nh.get_has_value(mass_, frame)
    && nh.get_has_value(element_, frame);
    }
    IMP_RMF_SHOWABLE(AtomConstFactory, "AtomConstFactory");
    };

    typedef vector<AtomConstFactory> AtomConstFactories;

/** Information regarding a chain.

       \see Chain
       \see ChainConstFactory
     */
    class ChainConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class ChainConstFactory;
    private:
    IndexKey chain_id_;
    ChainConst(NodeConstHandle nh,
                      unsigned int frame,
                  IndexKey chain_id): nh_(nh),
                                      frame_(frame),
                                     chain_id_(chain_id) {
    ;
    }
    public:
    Index get_chain_id() const {
  return nh_.get_value(chain_id_, frame_);
}
    IMP_RMF_SHOWABLE(ConstChain,
                     "ChainConst "
                     << nh_.get_name());
    ~ChainConst() {
    }
    };

    typedef vector<ChainConst> ChainConsts;

/** Information regarding a chain.

       \see ChainConst
       \see ChainFactory
     */
    class Chain {
    NodeHandle nh_;
    unsigned int frame_;
    friend class ChainFactory;
    private:
    IndexKey chain_id_;
    Chain(NodeHandle nh,
                      unsigned int frame,
                  IndexKey chain_id): nh_(nh),
                                      frame_(frame),
                                     chain_id_(chain_id) {
    ;
    }
    public:
    Index get_chain_id() const {
  return nh_.get_value(chain_id_, frame_);
}
void set_chain_id(Index v) {
   nh_.set_value(chain_id_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstChain,
                     "Chain "
                     << nh_.get_name());
    ~Chain() {
    }
    };

    typedef vector<Chain> Chains;

/** Create decorators of type Chain.

       \see Chain
       \see ChainConstFactory
    */
    class ChainFactory {
    private:
    IndexKey chain_id_;
    public:
    typedef FileHandle File;
    typedef Chain Decorator;
    ChainFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
chain_id_=get_key_always<IndexTraits>(fh, cat,
                               "chain id", false);
};
    }
    Chain get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Chain(nh, frame, chain_id_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(chain_id_, frame);
    }
    IMP_RMF_SHOWABLE(ChainFactory, "ChainFactory");
    };

    typedef vector<ChainFactory> ChainFactories;

/** Create decorators of type Chain.

       \see ChainConst
       \see ChainFactory
    */
    class ChainConstFactory {
    private:
    IndexKey chain_id_;
    public:
    typedef FileConstHandle File;
    typedef ChainConst Decorator;
    ChainConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
chain_id_=(fh.get_has_key<IndexTraits>(cat, "chain id")?
                   fh.get_key<IndexTraits>(cat, "chain id")
                              :IndexKey());
};
    }
    ChainConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ChainConst(nh, frame, chain_id_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(chain_id_, frame);
    }
    IMP_RMF_SHOWABLE(ChainConstFactory, "ChainConstFactory");
    };

    typedef vector<ChainConstFactory> ChainConstFactories;

/** Information regarding a fragment of a molecule.

       \see Domain
       \see DomainConstFactory
     */
    class DomainConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class DomainConstFactory;
    private:
    boost::array<IntKey,2> indexes_;
    DomainConst(NodeConstHandle nh,
                      unsigned int frame,
                  boost::array<IntKey, 2> indexes): nh_(nh),
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

    typedef vector<DomainConst> DomainConsts;

/** Information regarding a fragment of a molecule.

       \see DomainConst
       \see DomainFactory
     */
    class Domain {
    NodeHandle nh_;
    unsigned int frame_;
    friend class DomainFactory;
    private:
    boost::array<IntKey,2> indexes_;
    Domain(NodeHandle nh,
                      unsigned int frame,
                  boost::array<IntKey, 2> indexes): nh_(nh),
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

    typedef vector<Domain> Domains;

/** Create decorators of type Domain.

       \see Domain
       \see DomainConstFactory
    */
    class DomainFactory {
    private:
    boost::array<IntKey,2> indexes_;
    public:
    typedef FileHandle File;
    typedef Domain Decorator;
    DomainFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
indexes_[0]=get_key_always<IntTraits>(fh, cat,
                               "first residue index", false);
indexes_[1]=get_key_always<IntTraits>(fh, cat,
                               "last residue index", false);
};
    }
    Domain get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Domain(nh, frame, indexes_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(indexes_[0], frame)
  && nh.get_has_value(indexes_[1], frame)
  && nh.get_value(indexes_[0], frame)
   <nh.get_value(indexes_[1], frame);
    }
    IMP_RMF_SHOWABLE(DomainFactory, "DomainFactory");
    };

    typedef vector<DomainFactory> DomainFactories;

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
    DomainConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
indexes_[0]=(fh.get_has_key<IntTraits>(cat, "first residue index")?
                   fh.get_key<IntTraits>(cat, "first residue index")
                              :IntKey());
indexes_[1]=(fh.get_has_key<IntTraits>(cat, "last residue index")?
                   fh.get_key<IntTraits>(cat, "last residue index")
                              :IntKey());
};
    }
    DomainConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return DomainConst(nh, frame, indexes_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(indexes_[0], frame)
  && nh.get_has_value(indexes_[1], frame)
  && nh.get_value(indexes_[0], frame)
   <nh.get_value(indexes_[1], frame);
    }
    IMP_RMF_SHOWABLE(DomainConstFactory, "DomainConstFactory");
    };

    typedef vector<DomainConstFactory> DomainConstFactories;

/** Information regarding a copy of a molecule.

       \see Copy
       \see CopyConstFactory
     */
    class CopyConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class CopyConstFactory;
    private:
    IndexKey copy_index_;
    CopyConst(NodeConstHandle nh,
                      unsigned int frame,
                  IndexKey copy_index): nh_(nh),
                                      frame_(frame),
                                     copy_index_(copy_index) {
    ;
    }
    public:
    Index get_copy_index() const {
  return nh_.get_value(copy_index_, frame_);
}
    IMP_RMF_SHOWABLE(ConstCopy,
                     "CopyConst "
                     << nh_.get_name());
    ~CopyConst() {
    }
    };

    typedef vector<CopyConst> CopyConsts;

/** Information regarding a copy of a molecule.

       \see CopyConst
       \see CopyFactory
     */
    class Copy {
    NodeHandle nh_;
    unsigned int frame_;
    friend class CopyFactory;
    private:
    IndexKey copy_index_;
    Copy(NodeHandle nh,
                      unsigned int frame,
                  IndexKey copy_index): nh_(nh),
                                      frame_(frame),
                                     copy_index_(copy_index) {
    ;
    }
    public:
    Index get_copy_index() const {
  return nh_.get_value(copy_index_, frame_);
}
void set_copy_index(Index v) {
   nh_.set_value(copy_index_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstCopy,
                     "Copy "
                     << nh_.get_name());
    ~Copy() {
    }
    };

    typedef vector<Copy> Copys;

/** Create decorators of type Copy.

       \see Copy
       \see CopyConstFactory
    */
    class CopyFactory {
    private:
    IndexKey copy_index_;
    public:
    typedef FileHandle File;
    typedef Copy Decorator;
    CopyFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
copy_index_=get_key_always<IndexTraits>(fh, cat,
                               "copy index", false);
};
    }
    Copy get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Copy(nh, frame, copy_index_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(copy_index_, frame);
    }
    IMP_RMF_SHOWABLE(CopyFactory, "CopyFactory");
    };

    typedef vector<CopyFactory> CopyFactories;

/** Create decorators of type Copy.

       \see CopyConst
       \see CopyFactory
    */
    class CopyConstFactory {
    private:
    IndexKey copy_index_;
    public:
    typedef FileConstHandle File;
    typedef CopyConst Decorator;
    CopyConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
copy_index_=(fh.get_has_key<IndexTraits>(cat, "copy index")?
                   fh.get_key<IndexTraits>(cat, "copy index")
                              :IndexKey());
};
    }
    CopyConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return CopyConst(nh, frame, copy_index_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(copy_index_, frame);
    }
    IMP_RMF_SHOWABLE(CopyConstFactory, "CopyConstFactory");
    };

    typedef vector<CopyConstFactory> CopyConstFactories;

/** Information regarding diffusion coefficients.

       \see Diffuser
       \see DiffuserConstFactory
     */
    class DiffuserConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class DiffuserConstFactory;
    private:
    FloatKey diffusion_coefficient_;
    DiffuserConst(NodeConstHandle nh,
                      unsigned int frame,
                  FloatKey diffusion_coefficient): nh_(nh),
                                      frame_(frame),
                               diffusion_coefficient_(diffusion_coefficient) {
    ;
    }
    public:
    Float get_diffusion_coefficient() const {
  return nh_.get_value(diffusion_coefficient_, frame_);
}
    IMP_RMF_SHOWABLE(ConstDiffuser,
                     "DiffuserConst "
                     << nh_.get_name());
    ~DiffuserConst() {
    }
    };

    typedef vector<DiffuserConst> DiffuserConsts;

/** Information regarding diffusion coefficients.

       \see DiffuserConst
       \see DiffuserFactory
     */
    class Diffuser {
    NodeHandle nh_;
    unsigned int frame_;
    friend class DiffuserFactory;
    private:
    FloatKey diffusion_coefficient_;
    Diffuser(NodeHandle nh,
                      unsigned int frame,
                  FloatKey diffusion_coefficient): nh_(nh),
                                      frame_(frame),
                               diffusion_coefficient_(diffusion_coefficient) {
    ;
    }
    public:
    Float get_diffusion_coefficient() const {
  return nh_.get_value(diffusion_coefficient_, frame_);
}
void set_diffusion_coefficient(Float v) {
   nh_.set_value(diffusion_coefficient_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstDiffuser,
                     "Diffuser "
                     << nh_.get_name());
    ~Diffuser() {
    }
    };

    typedef vector<Diffuser> Diffusers;

/** Create decorators of type Diffuser.

       \see Diffuser
       \see DiffuserConstFactory
    */
    class DiffuserFactory {
    private:
    FloatKey diffusion_coefficient_;
    public:
    typedef FileHandle File;
    typedef Diffuser Decorator;
    DiffuserFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "physics");
diffusion_coefficient_=get_key_always<FloatTraits>(fh, cat,
                               "diffusion coefficient", false);
};
    }
    Diffuser get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Diffuser(nh, frame, diffusion_coefficient_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(diffusion_coefficient_, frame);
    }
    IMP_RMF_SHOWABLE(DiffuserFactory, "DiffuserFactory");
    };

    typedef vector<DiffuserFactory> DiffuserFactories;

/** Create decorators of type Diffuser.

       \see DiffuserConst
       \see DiffuserFactory
    */
    class DiffuserConstFactory {
    private:
    FloatKey diffusion_coefficient_;
    public:
    typedef FileConstHandle File;
    typedef DiffuserConst Decorator;
    DiffuserConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("physics");
diffusion_coefficient_=(fh.get_has_key<FloatTraits>(cat,
                                                    "diffusion coefficient")?
                   fh.get_key<FloatTraits>(cat, "diffusion coefficient")
                              :FloatKey());
};
    }
    DiffuserConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return DiffuserConst(nh, frame, diffusion_coefficient_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(diffusion_coefficient_, frame);
    }
    IMP_RMF_SHOWABLE(DiffuserConstFactory, "DiffuserConstFactory");
    };

    typedef vector<DiffuserConstFactory> DiffuserConstFactories;

/** A numeric tag for keeping track of types of molecules.

       \see Typed
       \see TypedConstFactory
     */
    class TypedConst {
    NodeConstHandle nh_;
    unsigned int frame_;
    friend class TypedConstFactory;
    private:
    StringKey type_name_;
    TypedConst(NodeConstHandle nh,
                      unsigned int frame,
                  StringKey type_name): nh_(nh),
                                      frame_(frame),
                                     type_name_(type_name) {
    ;
    }
    public:
    String get_type_name() const {
  return nh_.get_value(type_name_, frame_);
}
    IMP_RMF_SHOWABLE(ConstTyped,
                     "TypedConst "
                     << nh_.get_name());
    ~TypedConst() {
    }
    };

    typedef vector<TypedConst> TypedConsts;

/** A numeric tag for keeping track of types of molecules.

       \see TypedConst
       \see TypedFactory
     */
    class Typed {
    NodeHandle nh_;
    unsigned int frame_;
    friend class TypedFactory;
    private:
    StringKey type_name_;
    Typed(NodeHandle nh,
                      unsigned int frame,
                  StringKey type_name): nh_(nh),
                                      frame_(frame),
                                     type_name_(type_name) {
    ;
    }
    public:
    String get_type_name() const {
  return nh_.get_value(type_name_, frame_);
}
void set_type_name(String v) {
   nh_.set_value(type_name_, v, frame_);
}
    IMP_RMF_SHOWABLE(ConstTyped,
                     "Typed "
                     << nh_.get_name());
    ~Typed() {
    }
    };

    typedef vector<Typed> Typeds;

/** Create decorators of type Typed.

       \see Typed
       \see TypedConstFactory
    */
    class TypedFactory {
    private:
    StringKey type_name_;
    public:
    typedef FileHandle File;
    typedef Typed Decorator;
    TypedFactory(FileHandle fh){
    {
  CategoryD<1> cat=get_category_always<1>(fh, "sequence");
type_name_=get_key_always<StringTraits>(fh, cat,
                               "type name", false);
};
    }
    Typed get(NodeHandle nh, unsigned int frame=0) const {
      ;
      return Typed(nh, frame, type_name_);
    }
    bool get_is(NodeHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(type_name_, frame);
    }
    IMP_RMF_SHOWABLE(TypedFactory, "TypedFactory");
    };

    typedef vector<TypedFactory> TypedFactories;

/** Create decorators of type Typed.

       \see TypedConst
       \see TypedFactory
    */
    class TypedConstFactory {
    private:
    StringKey type_name_;
    public:
    typedef FileConstHandle File;
    typedef TypedConst Decorator;
    TypedConstFactory(FileConstHandle fh){
    {
  CategoryD<1> cat=fh.get_category<1>("sequence");
type_name_=(fh.get_has_key<StringTraits>(cat, "type name")?
                   fh.get_key<StringTraits>(cat, "type name")
                              :StringKey());
};
    }
    TypedConst get(NodeConstHandle nh, unsigned int frame=0) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return TypedConst(nh, frame, type_name_);
    }
    bool get_is(NodeConstHandle nh, unsigned int frame=0) const {
      return nh.get_has_value(type_name_, frame);
    }
    IMP_RMF_SHOWABLE(TypedConstFactory, "TypedConstFactory");
    };

    typedef vector<TypedConstFactory> TypedConstFactories;

} /* namespace RMF */

#endif /* IMPLIBRMF_DECORATORS_H */
