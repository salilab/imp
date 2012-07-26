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
#include "Decorator.h"
#include "Factory.h"
#include "constants.h"
#include "internal/utility.h"
#include "internal/lazy.h"
namespace RMF {

  /** These particles has associated color information.

      \see Colored
      \see ColoredConstFactory
  */
  class ColoredConst:
    public Decorator<ColoredConst, NodeConstHandle> {
    friend class ColoredConstFactory;
  private:
    typedef Decorator<ColoredConst, NodeConstHandle> P;
    FloatKeys rgb_color_;
    FloatKeys rgb_color_pf_;
    ColoredConst(NodeConstHandle nh,
                 int frame,
                 FloatKeys rgb_color,
                 FloatKeys rgb_color_pf):
      P(nh, frame),
      rgb_color_(rgb_color),
      rgb_color_pf_(rgb_color_pf) {
      ;
    }
  public:
    Floats get_rgb_color() const {
      return P::get_values(rgb_color_, rgb_color_pf_);
    }
    static std::string get_decorator_type_name() {
      return "ColoredConst";
    }
  };

  typedef vector<ColoredConst>
  ColoredConsts;

  /** These particles has associated color information.

      \see ColoredConst
      \see ColoredFactory
  */
  class Colored:
    public Decorator<Colored, NodeHandle> {
    friend class ColoredFactory;
  private:
    typedef Decorator<Colored, NodeHandle> P;
    internal::FloatLazyKeys rgb_color_;
    internal::FloatLazyKeys rgb_color_pf_;
    Colored(NodeHandle nh,
            int frame,
            internal::FloatLazyKeys rgb_color,
            internal::FloatLazyKeys rgb_color_pf):
      P(nh, frame),
      rgb_color_(rgb_color),
      rgb_color_pf_(rgb_color_pf) {
      ;
    }
  public:
    Floats get_rgb_color() const {
      return P::get_values(rgb_color_, rgb_color_pf_);
    }
    void set_rgb_color(const Floats &v) {
      P::set_values(rgb_color_, rgb_color_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Colored";
    }
  };

  typedef vector<Colored>
  Coloreds;

  /** Create decorators of type Colored.

      \see Colored
      \see ColoredConstFactory
  */
  class ColoredFactory:
    public Factory<Colored>
  {
  private:
    typedef Factory<Colored> P;
    internal::FloatLazyKeys rgb_color_;
    internal::FloatLazyKeys rgb_color_pf_;
  public:
    ColoredFactory(FileHandle fh):
      P(){
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
                int frame=ALL_FRAMES) const {
      ;
      return Colored(nh, frame, rgb_color_,
                     rgb_color_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, rgb_color_,
                                rgb_color_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, rgb_color_,
                                 rgb_color_pf_, frame);
      }
    }
  };

  typedef vector<ColoredFactory>
  ColoredFactories;

  /** Create decorators of type Colored.

      \see ColoredConst
      \see ColoredFactory
  */
  class ColoredConstFactory:
    public Factory<ColoredConst>
  {
  private:
    typedef Factory<ColoredConst> P;
    FloatKeys rgb_color_;
    FloatKeys rgb_color_pf_;
  public:
    ColoredConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  rgb_color_names;
        rgb_color_names.push_back("rgb color red");
        rgb_color_names.push_back("rgb color green");
        rgb_color_names.push_back("rgb color blue");
        rgb_color_=P::get_keys<FloatTraits>(fh,
                                            cat,
                                            rgb_color_names,
                                            false);
        rgb_color_pf_=P::get_keys<FloatTraits>(fh,
                                               cat,
                                               rgb_color_names,
                                               true);
      };
    }
    ColoredConst get(NodeConstHandle nh,
                     int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ColoredConst(nh, frame, rgb_color_,
                          rgb_color_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, rgb_color_,
                                rgb_color_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, rgb_color_,
                                 rgb_color_pf_, frame);
      }
    }
  };

  typedef vector<ColoredConstFactory>
  ColoredConstFactories;

  /** These particles has associated coordinates and radius information.

      \see Particle
      \see ParticleConstFactory
  */
  class ParticleConst:
    public Decorator<ParticleConst, NodeConstHandle> {
    friend class ParticleConstFactory;
  private:
    typedef Decorator<ParticleConst, NodeConstHandle> P;
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
      P(nh, frame),
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
      return P::get_values(coordinates_, coordinates_pf_);
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    Float get_mass() const {
      return P::get_value(mass_,
                          mass_pf_);
    }
    Floats get_all_masss() const {
      return P::get_all_values(mass_pf_);
    }
    static std::string get_decorator_type_name() {
      return "ParticleConst";
    }
  };

  typedef vector<ParticleConst>
  ParticleConsts;

  /** These particles has associated coordinates and radius information.

      \see ParticleConst
      \see ParticleFactory
  */
  class Particle:
    public Decorator<Particle, NodeHandle> {
    friend class ParticleFactory;
  private:
    typedef Decorator<Particle, NodeHandle> P;
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
      P(nh, frame),
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
      return P::get_values(coordinates_, coordinates_pf_);
    }
    void set_coordinates(const Floats &v) {
      P::set_values(coordinates_, coordinates_pf_, v);
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    void set_radius(Float v) {
      P::set_value(radius_,
                   radius_pf_, v);
    }
    Float get_mass() const {
      return P::get_value(mass_,
                          mass_pf_);
    }
    Floats get_all_masss() const {
      return P::get_all_values(mass_pf_);
    }
    void set_mass(Float v) {
      P::set_value(mass_,
                   mass_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Particle";
    }
  };

  typedef vector<Particle>
  Particles;

  /** Create decorators of type Particle.

      \see Particle
      \see ParticleConstFactory
  */
  class ParticleFactory:
    public Factory<Particle>
  {
  private:
    typedef Factory<Particle> P;
    internal::FloatLazyKeys coordinates_;
    internal::FloatLazyKeys coordinates_pf_;
    internal::FloatLazyKey radius_;
    internal::FloatLazyKey radius_pf_;
    internal::FloatLazyKey mass_;
    internal::FloatLazyKey mass_pf_;
  public:
    ParticleFactory(FileHandle fh):
      P(){
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
                 int frame=ALL_FRAMES) const {
      ;
      return Particle(nh, frame, coordinates_,
                      coordinates_pf_,
                      radius_,
                      radius_pf_,
                      mass_,
                      mass_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, mass_,
                                  mass_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, mass_,
                              mass_pf_, frame);
      }
    }
  };

  typedef vector<ParticleFactory>
  ParticleFactories;

  /** Create decorators of type Particle.

      \see ParticleConst
      \see ParticleFactory
  */
  class ParticleConstFactory:
    public Factory<ParticleConst>
  {
  private:
    typedef Factory<ParticleConst> P;
    FloatKeys coordinates_;
    FloatKeys coordinates_pf_;
    FloatKey radius_;
    FloatKey radius_pf_;
    FloatKey mass_;
    FloatKey mass_pf_;
  public:
    ParticleConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
        coordinates_=P::get_keys<FloatTraits>(fh,
                                              cat,
                                              coordinates_names,
                                              false);
        coordinates_pf_=P::get_keys<FloatTraits>(fh,
                                                 cat,
                                                 coordinates_names,
                                                 true);
        radius_=P::get_key<FloatTraits>(fh,
                                        cat,
                                        "radius",
                                        false);
        radius_pf_=P::get_key<FloatTraits>(fh,
                                           cat,
                                           "radius",
                                           true);
        mass_=P::get_key<FloatTraits>(fh,
                                      cat,
                                      "mass",
                                      false);
        mass_pf_=P::get_key<FloatTraits>(fh,
                                         cat,
                                         "mass",
                                         true);
      };
    }
    ParticleConst get(NodeConstHandle nh,
                      int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ParticleConst(nh, frame, coordinates_,
                           coordinates_pf_,
                           radius_,
                           radius_pf_,
                           mass_,
                           mass_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, mass_,
                                  mass_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, mass_,
                              mass_pf_, frame);
      }
    }
  };

  typedef vector<ParticleConstFactory>
  ParticleConstFactories;

  /** These particles has associated coordinates and radius information.

      \see IntermediateParticle
      \see IntermediateParticleConstFactory
  */
  class IntermediateParticleConst:
    public Decorator<IntermediateParticleConst, NodeConstHandle> {
    friend class IntermediateParticleConstFactory;
  private:
    typedef Decorator<IntermediateParticleConst, NodeConstHandle> P;
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
      P(nh, frame),
      coordinates_(coordinates),
      coordinates_pf_(coordinates_pf),
      radius_(radius),
      radius_pf_(radius_pf) {
      ;
    }
  public:
    Floats get_coordinates() const {
      return P::get_values(coordinates_, coordinates_pf_);
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    static std::string get_decorator_type_name() {
      return "IntermediateParticleConst";
    }
  };

  typedef vector<IntermediateParticleConst>
  IntermediateParticleConsts;

  /** These particles has associated coordinates and radius information.

      \see IntermediateParticleConst
      \see IntermediateParticleFactory
  */
  class IntermediateParticle:
    public Decorator<IntermediateParticle, NodeHandle> {
    friend class IntermediateParticleFactory;
  private:
    typedef Decorator<IntermediateParticle, NodeHandle> P;
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
      P(nh, frame),
      coordinates_(coordinates),
      coordinates_pf_(coordinates_pf),
      radius_(radius),
      radius_pf_(radius_pf) {
      ;
    }
  public:
    Floats get_coordinates() const {
      return P::get_values(coordinates_, coordinates_pf_);
    }
    void set_coordinates(const Floats &v) {
      P::set_values(coordinates_, coordinates_pf_, v);
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    void set_radius(Float v) {
      P::set_value(radius_,
                   radius_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "IntermediateParticle";
    }
  };

  typedef vector<IntermediateParticle>
  IntermediateParticles;

  /** Create decorators of type IntermediateParticle.

      \see IntermediateParticle
      \see IntermediateParticleConstFactory
  */
  class IntermediateParticleFactory:
    public Factory<IntermediateParticle>
  {
  private:
    typedef Factory<IntermediateParticle> P;
    internal::FloatLazyKeys coordinates_;
    internal::FloatLazyKeys coordinates_pf_;
    internal::FloatLazyKey radius_;
    internal::FloatLazyKey radius_pf_;
  public:
    IntermediateParticleFactory(FileHandle fh):
      P(){
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
                             int frame=ALL_FRAMES) const {
      ;
      return IntermediateParticle(nh, frame, coordinates_,
                                  coordinates_pf_,
                                  radius_,
                                  radius_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame);
      }
    }
  };

  typedef vector<IntermediateParticleFactory>
  IntermediateParticleFactories;

  /** Create decorators of type IntermediateParticle.

      \see IntermediateParticleConst
      \see IntermediateParticleFactory
  */
  class IntermediateParticleConstFactory:
    public Factory<IntermediateParticleConst>
  {
  private:
    typedef Factory<IntermediateParticleConst> P;
    FloatKeys coordinates_;
    FloatKeys coordinates_pf_;
    FloatKey radius_;
    FloatKey radius_pf_;
  public:
    IntermediateParticleConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
        coordinates_=P::get_keys<FloatTraits>(fh,
                                              cat,
                                              coordinates_names,
                                              false);
        coordinates_pf_=P::get_keys<FloatTraits>(fh,
                                                 cat,
                                                 coordinates_names,
                                                 true);
        radius_=P::get_key<FloatTraits>(fh,
                                        cat,
                                        "radius",
                                        false);
        radius_pf_=P::get_key<FloatTraits>(fh,
                                           cat,
                                           "radius",
                                           true);
      };
    }
    IntermediateParticleConst get(NodeConstHandle nh,
                                  int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return IntermediateParticleConst(nh, frame, coordinates_,
                                       coordinates_pf_,
                                       radius_,
                                       radius_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame);
      }
    }
  };

  typedef vector<IntermediateParticleConstFactory>
  IntermediateParticleConstFactories;

  /** These particles has associated coordinates and orientation information.

      \see RigidParticle
      \see RigidParticleConstFactory
  */
  class RigidParticleConst:
    public Decorator<RigidParticleConst, NodeConstHandle> {
    friend class RigidParticleConstFactory;
  private:
    typedef Decorator<RigidParticleConst, NodeConstHandle> P;
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
      P(nh, frame),
      orientation_(orientation),
      orientation_pf_(orientation_pf),
      coordinates_(coordinates),
      coordinates_pf_(coordinates_pf) {
      ;
    }
  public:
    Floats get_orientation() const {
      return P::get_values(orientation_, orientation_pf_);
    }
    Floats get_coordinates() const {
      return P::get_values(coordinates_, coordinates_pf_);
    }
    static std::string get_decorator_type_name() {
      return "RigidParticleConst";
    }
  };

  typedef vector<RigidParticleConst>
  RigidParticleConsts;

  /** These particles has associated coordinates and orientation information.

      \see RigidParticleConst
      \see RigidParticleFactory
  */
  class RigidParticle:
    public Decorator<RigidParticle, NodeHandle> {
    friend class RigidParticleFactory;
  private:
    typedef Decorator<RigidParticle, NodeHandle> P;
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
      P(nh, frame),
      orientation_(orientation),
      orientation_pf_(orientation_pf),
      coordinates_(coordinates),
      coordinates_pf_(coordinates_pf) {
      ;
    }
  public:
    Floats get_orientation() const {
      return P::get_values(orientation_, orientation_pf_);
    }
    void set_orientation(const Floats &v) {
      P::set_values(orientation_, orientation_pf_, v);
    }
    Floats get_coordinates() const {
      return P::get_values(coordinates_, coordinates_pf_);
    }
    void set_coordinates(const Floats &v) {
      P::set_values(coordinates_, coordinates_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "RigidParticle";
    }
  };

  typedef vector<RigidParticle>
  RigidParticles;

  /** Create decorators of type RigidParticle.

      \see RigidParticle
      \see RigidParticleConstFactory
  */
  class RigidParticleFactory:
    public Factory<RigidParticle>
  {
  private:
    typedef Factory<RigidParticle> P;
    internal::FloatLazyKeys orientation_;
    internal::FloatLazyKeys orientation_pf_;
    internal::FloatLazyKeys coordinates_;
    internal::FloatLazyKeys coordinates_pf_;
  public:
    RigidParticleFactory(FileHandle fh):
      P(){
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
                      int frame=ALL_FRAMES) const {
      ;
      return RigidParticle(nh, frame, orientation_,
                           orientation_pf_,
                           coordinates_,
                           coordinates_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, orientation_,
                                orientation_pf_, frame)
              && P::get_has_values(nh, coordinates_,
                                   coordinates_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, orientation_,
                                 orientation_pf_, frame)
          && P::get_has_values(nh, coordinates_,
                               coordinates_pf_, frame);
      }
    }
  };

  typedef vector<RigidParticleFactory>
  RigidParticleFactories;

  /** Create decorators of type RigidParticle.

      \see RigidParticleConst
      \see RigidParticleFactory
  */
  class RigidParticleConstFactory:
    public Factory<RigidParticleConst>
  {
  private:
    typedef Factory<RigidParticleConst> P;
    FloatKeys orientation_;
    FloatKeys orientation_pf_;
    FloatKeys coordinates_;
    FloatKeys coordinates_pf_;
  public:
    RigidParticleConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  orientation_names;
        orientation_names.push_back("orientation r");
        orientation_names.push_back("orientation i");
        orientation_names.push_back("orientation j");
        orientation_names.push_back("orientation k");
        orientation_=P::get_keys<FloatTraits>(fh,
                                              cat,
                                              orientation_names,
                                              false);
        orientation_pf_=P::get_keys<FloatTraits>(fh,
                                                 cat,
                                                 orientation_names,
                                                 true);
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
        coordinates_=P::get_keys<FloatTraits>(fh,
                                              cat,
                                              coordinates_names,
                                              false);
        coordinates_pf_=P::get_keys<FloatTraits>(fh,
                                                 cat,
                                                 coordinates_names,
                                                 true);
      };
    }
    RigidParticleConst get(NodeConstHandle nh,
                           int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return RigidParticleConst(nh, frame, orientation_,
                                orientation_pf_,
                                coordinates_,
                                coordinates_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, orientation_,
                                orientation_pf_, frame)
              && P::get_has_values(nh, coordinates_,
                                   coordinates_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, orientation_,
                                 orientation_pf_, frame)
          && P::get_has_values(nh, coordinates_,
                               coordinates_pf_, frame);
      }
    }
  };

  typedef vector<RigidParticleConstFactory>
  RigidParticleConstFactories;

  /** A geometric ball.

      \see Ball
      \see BallConstFactory
  */
  class BallConst:
    public Decorator<BallConst, NodeConstHandle> {
    friend class BallConstFactory;
  private:
    typedef Decorator<BallConst, NodeConstHandle> P;
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
      P(nh, frame),
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
      if (!coordinates_.empty()
          && get_node().get_has_value(coordinates_[0])) {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_[i]);
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_pf_[i], get_frame());
        }
      }
      return ret;
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    static std::string get_decorator_type_name() {
      return "BallConst";
    }
  };

  typedef vector<BallConst>
  BallConsts;

  /** A geometric ball.

      \see BallConst
      \see BallFactory
  */
  class Ball:
    public Decorator<Ball, NodeHandle> {
    friend class BallFactory;
  private:
    typedef Decorator<Ball, NodeHandle> P;
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
      P(nh, frame),
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
      if (get_node().get_has_value(coordinates_[0])) {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_[i]);
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_pf_[i], get_frame());
        }
      }
      return ret;
    }
    void set_coordinates(const Floats &v) {
      if (get_frame()>=0) {
        for (unsigned int i=0; i< 3; ++i) {
          get_node().set_value(coordinates_pf_[i], v[i], get_frame());
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          get_node().set_value(coordinates_[i], v[i]);
        }
      }
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    void set_radius(Float v) {
      P::set_value(radius_,
                   radius_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Ball";
    }
  };

  typedef vector<Ball>
  Balls;

  /** Create decorators of type Ball.

      \see Ball
      \see BallConstFactory
  */
  class BallFactory:
    public Factory<Ball>
  {
  private:
    typedef Factory<Ball> P;
    internal::FloatLazyKeys coordinates_;
    internal::FloatLazyKeys coordinates_pf_;
    internal::FloatLazyKey radius_;
    internal::FloatLazyKey radius_pf_;
    internal::IndexLazyKey type_;
    internal::IndexLazyKey type_pf_;
  public:
    BallFactory(FileHandle fh):
      P(){
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
             int frame=ALL_FRAMES) const {
      ;
      return Ball(nh, frame, coordinates_,
                  coordinates_pf_,
                  radius_,
                  radius_pf_,
                  type_,
                  type_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)
              && nh.get_value(type_)==0) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame)
          && nh.get_value(type_)==0;
      }
    }
  };

  typedef vector<BallFactory>
  BallFactories;

  /** Create decorators of type Ball.

      \see BallConst
      \see BallFactory
  */
  class BallConstFactory:
    public Factory<BallConst>
  {
  private:
    typedef Factory<BallConst> P;
    FloatKeys coordinates_;
    FloatKeys coordinates_pf_;
    FloatKey radius_;
    FloatKey radius_pf_;
    IndexKey type_;
    IndexKey type_pf_;
  public:
    BallConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
        coordinates_=P::get_keys<FloatTraits>(fh,
                                              cat,
                                              coordinates_names,
                                              false);
        coordinates_pf_=P::get_keys<FloatTraits>(fh,
                                                 cat,
                                                 coordinates_names,
                                                 true);
        radius_=P::get_key<FloatTraits>(fh,
                                        cat,
                                        "radius",
                                        false);
        radius_pf_=P::get_key<FloatTraits>(fh,
                                           cat,
                                           "radius",
                                           true);
        type_=P::get_key<IndexTraits>(fh,
                                      cat,
                                      "type",
                                      false);
        type_pf_=P::get_key<IndexTraits>(fh,
                                         cat,
                                         "type",
                                         true);
      };
    }
    BallConst get(NodeConstHandle nh,
                  int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return BallConst(nh, frame, coordinates_,
                       coordinates_pf_,
                       radius_,
                       radius_pf_,
                       type_,
                       type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)
              && nh.get_value(type_)==0) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame)
          && nh.get_value(type_)==0;
      }
    }
  };

  typedef vector<BallConstFactory>
  BallConstFactories;

  /** A geometric cylinder.

      \see Cylinder
      \see CylinderConstFactory
  */
  class CylinderConst:
    public Decorator<CylinderConst, NodeConstHandle> {
    friend class CylinderConstFactory;
  private:
    typedef Decorator<CylinderConst, NodeConstHandle> P;
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
      P(nh, frame),
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
      if (!coordinates_.empty() && get_node().get_has_value(coordinates_[0])) {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_[i]);
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_pf_[i], get_frame());
        }
      }
      return ret;
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    static std::string get_decorator_type_name() {
      return "CylinderConst";
    }
  };

  typedef vector<CylinderConst>
  CylinderConsts;

  /** A geometric cylinder.

      \see CylinderConst
      \see CylinderFactory
  */
  class Cylinder:
    public Decorator<Cylinder, NodeHandle> {
    friend class CylinderFactory;
  private:
    typedef Decorator<Cylinder, NodeHandle> P;
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
      P(nh, frame),
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
      if (get_node().get_has_value(coordinates_[0])) {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_[i]);
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_pf_[i], get_frame());
        }
      }
      return ret;
    }
    void set_coordinates(const FloatsList &v) {
      if (get_frame()>=0) {
        for (unsigned int i=0; i< 3; ++i) {
          get_node().set_value(coordinates_pf_[i], v[i], get_frame());
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          get_node().set_value(coordinates_[i], v[i]);
        }
      }
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    void set_radius(Float v) {
      P::set_value(radius_,
                   radius_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Cylinder";
    }
  };

  typedef vector<Cylinder>
  Cylinders;

  /** Create decorators of type Cylinder.

      \see Cylinder
      \see CylinderConstFactory
  */
  class CylinderFactory:
    public Factory<Cylinder>
  {
  private:
    typedef Factory<Cylinder> P;
    internal::FloatsLazyKeys coordinates_;
    internal::FloatsLazyKeys coordinates_pf_;
    internal::FloatLazyKey radius_;
    internal::FloatLazyKey radius_pf_;
    internal::IndexLazyKey type_;
    internal::IndexLazyKey type_pf_;
  public:
    CylinderFactory(FileHandle fh):
      P(){
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
                 int frame=ALL_FRAMES) const {
      ;
      return Cylinder(nh, frame, coordinates_,
                      coordinates_pf_,
                      radius_,
                      radius_pf_,
                      type_,
                      type_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)
              && nh.get_value(type_)==1) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame)
          && nh.get_value(type_)==1;
      }
    }
  };

  typedef vector<CylinderFactory>
  CylinderFactories;

  /** Create decorators of type Cylinder.

      \see CylinderConst
      \see CylinderFactory
  */
  class CylinderConstFactory:
    public Factory<CylinderConst>
  {
  private:
    typedef Factory<CylinderConst> P;
    FloatsKeys coordinates_;
    FloatsKeys coordinates_pf_;
    FloatKey radius_;
    FloatKey radius_pf_;
    IndexKey type_;
    IndexKey type_pf_;
  public:
    CylinderConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian xs");
        coordinates_names.push_back("cartesian ys");
        coordinates_names.push_back("cartesian zs");
        coordinates_=P::get_keys<FloatsTraits>(fh,
                                               cat,
                                               coordinates_names,
                                               false);
        coordinates_pf_=P::get_keys<FloatsTraits>(fh,
                                                  cat,
                                                  coordinates_names,
                                                  true);
        radius_=P::get_key<FloatTraits>(fh,
                                        cat,
                                        "radius",
                                        false);
        radius_pf_=P::get_key<FloatTraits>(fh,
                                           cat,
                                           "radius",
                                           true);
        type_=P::get_key<IndexTraits>(fh,
                                      cat,
                                      "type",
                                      false);
        type_pf_=P::get_key<IndexTraits>(fh,
                                         cat,
                                         "type",
                                         true);
      };
    }
    CylinderConst get(NodeConstHandle nh,
                      int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return CylinderConst(nh, frame, coordinates_,
                           coordinates_pf_,
                           radius_,
                           radius_pf_,
                           type_,
                           type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)
              && nh.get_value(type_)==1) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame)
          && nh.get_value(type_)==1;
      }
    }
  };

  typedef vector<CylinderConstFactory>
  CylinderConstFactories;

  /** A geometric line setgment.

      \see Segment
      \see SegmentConstFactory
  */
  class SegmentConst:
    public Decorator<SegmentConst, NodeConstHandle> {
    friend class SegmentConstFactory;
  private:
    typedef Decorator<SegmentConst, NodeConstHandle> P;
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
      P(nh, frame),
      coordinates_(coordinates),
      coordinates_pf_(coordinates_pf),
      type_(type),
      type_pf_(type_pf) {
      ;
    }
  public:
    FloatsList get_coordinates() const {
      FloatsList ret(3);
      if (!coordinates_.empty() && get_node().get_has_value(coordinates_[0])) {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_[i]);
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_pf_[i], get_frame());
        }
      }
      return ret;
    }
    static std::string get_decorator_type_name() {
      return "SegmentConst";
    }
  };

  typedef vector<SegmentConst>
  SegmentConsts;

  /** A geometric line setgment.

      \see SegmentConst
      \see SegmentFactory
  */
  class Segment:
    public Decorator<Segment, NodeHandle> {
    friend class SegmentFactory;
  private:
    typedef Decorator<Segment, NodeHandle> P;
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
      P(nh, frame),
      coordinates_(coordinates),
      coordinates_pf_(coordinates_pf),
      type_(type),
      type_pf_(type_pf) {
      nh.set_value(type_, 1);;
    }
  public:
    FloatsList get_coordinates() const {
      FloatsList ret(3);
      if (get_node().get_has_value(coordinates_[0])) {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_[i]);
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          ret[i]=get_node().get_value(coordinates_pf_[i], get_frame());
        }
      }
      return ret;
    }
    void set_coordinates(const FloatsList &v) {
      if (get_frame()>=0) {
        for (unsigned int i=0; i< 3; ++i) {
          get_node().set_value(coordinates_pf_[i], v[i], get_frame());
        }
      } else {
        for (unsigned int i=0; i< 3; ++i) {
          get_node().set_value(coordinates_[i], v[i]);
        }
      }
    }
    static std::string get_decorator_type_name() {
      return "Segment";
    }
  };

  typedef vector<Segment>
  Segments;

  /** Create decorators of type Segment.

      \see Segment
      \see SegmentConstFactory
  */
  class SegmentFactory:
    public Factory<Segment>
  {
  private:
    typedef Factory<Segment> P;
    internal::FloatsLazyKeys coordinates_;
    internal::FloatsLazyKeys coordinates_pf_;
    internal::IndexLazyKey type_;
    internal::IndexLazyKey type_pf_;
  public:
    SegmentFactory(FileHandle fh):
      P(){
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
                int frame=ALL_FRAMES) const {
      ;
      return Segment(nh, frame, coordinates_,
                     coordinates_pf_,
                     type_,
                     type_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)
              && nh.get_value(type_)==1) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame)
          && nh.get_value(type_)==1;
      }
    }
  };

  typedef vector<SegmentFactory>
  SegmentFactories;

  /** Create decorators of type Segment.

      \see SegmentConst
      \see SegmentFactory
  */
  class SegmentConstFactory:
    public Factory<SegmentConst>
  {
  private:
    typedef Factory<SegmentConst> P;
    FloatsKeys coordinates_;
    FloatsKeys coordinates_pf_;
    IndexKey type_;
    IndexKey type_pf_;
  public:
    SegmentConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("shape");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian xs");
        coordinates_names.push_back("cartesian ys");
        coordinates_names.push_back("cartesian zs");
        coordinates_=P::get_keys<FloatsTraits>(fh,
                                               cat,
                                               coordinates_names,
                                               false);
        coordinates_pf_=P::get_keys<FloatsTraits>(fh,
                                                  cat,
                                                  coordinates_names,
                                                  true);
        type_=P::get_key<IndexTraits>(fh,
                                      cat,
                                      "type",
                                      false);
        type_pf_=P::get_key<IndexTraits>(fh,
                                         cat,
                                         "type",
                                         true);
      };
    }
    SegmentConst get(NodeConstHandle nh,
                     int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return SegmentConst(nh, frame, coordinates_,
                          coordinates_pf_,
                          type_,
                          type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)
              && nh.get_value(type_)==1) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame)
          && nh.get_value(type_)==1;
      }
    }
  };

  typedef vector<SegmentConstFactory>
  SegmentConstFactories;

  /** Information regarding a publication.

      \see JournalArticle
      \see JournalArticleConstFactory
  */
  class JournalArticleConst:
    public Decorator<JournalArticleConst, NodeConstHandle> {
    friend class JournalArticleConstFactory;
  private:
    typedef Decorator<JournalArticleConst, NodeConstHandle> P;
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
      P(nh, frame),
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
      return P::get_value(title_,
                          title_pf_);
    }
    Strings get_all_titles() const {
      return P::get_all_values(title_pf_);
    }
    String get_journal() const {
      return P::get_value(journal_,
                          journal_pf_);
    }
    Strings get_all_journals() const {
      return P::get_all_values(journal_pf_);
    }
    String get_pubmed_id() const {
      return P::get_value(pubmed_id_,
                          pubmed_id_pf_);
    }
    Strings get_all_pubmed_ids() const {
      return P::get_all_values(pubmed_id_pf_);
    }
    Int get_year() const {
      return P::get_value(year_,
                          year_pf_);
    }
    Ints get_all_years() const {
      return P::get_all_values(year_pf_);
    }
    Strings get_authors() const {
      return P::get_value(authors_,
                          authors_pf_);
    }
    StringsList get_all_authorss() const {
      return P::get_all_values(authors_pf_);
    }
    static std::string get_decorator_type_name() {
      return "JournalArticleConst";
    }
  };

  typedef vector<JournalArticleConst>
  JournalArticleConsts;

  /** Information regarding a publication.

      \see JournalArticleConst
      \see JournalArticleFactory
  */
  class JournalArticle:
    public Decorator<JournalArticle, NodeHandle> {
    friend class JournalArticleFactory;
  private:
    typedef Decorator<JournalArticle, NodeHandle> P;
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
      P(nh, frame),
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
      return P::get_value(title_,
                          title_pf_);
    }
    Strings get_all_titles() const {
      return P::get_all_values(title_pf_);
    }
    void set_title(String v) {
      P::set_value(title_,
                   title_pf_, v);
    }
    String get_journal() const {
      return P::get_value(journal_,
                          journal_pf_);
    }
    Strings get_all_journals() const {
      return P::get_all_values(journal_pf_);
    }
    void set_journal(String v) {
      P::set_value(journal_,
                   journal_pf_, v);
    }
    String get_pubmed_id() const {
      return P::get_value(pubmed_id_,
                          pubmed_id_pf_);
    }
    Strings get_all_pubmed_ids() const {
      return P::get_all_values(pubmed_id_pf_);
    }
    void set_pubmed_id(String v) {
      P::set_value(pubmed_id_,
                   pubmed_id_pf_, v);
    }
    Int get_year() const {
      return P::get_value(year_,
                          year_pf_);
    }
    Ints get_all_years() const {
      return P::get_all_values(year_pf_);
    }
    void set_year(Int v) {
      P::set_value(year_,
                   year_pf_, v);
    }
    Strings get_authors() const {
      return P::get_value(authors_,
                          authors_pf_);
    }
    StringsList get_all_authorss() const {
      return P::get_all_values(authors_pf_);
    }
    void set_authors(Strings v) {
      P::set_value(authors_,
                   authors_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "JournalArticle";
    }
  };

  typedef vector<JournalArticle>
  JournalArticles;

  /** Create decorators of type JournalArticle.

      \see JournalArticle
      \see JournalArticleConstFactory
  */
  class JournalArticleFactory:
    public Factory<JournalArticle>
  {
  private:
    typedef Factory<JournalArticle> P;
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
    JournalArticleFactory(FileHandle fh):
      P(){
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
                       int frame=ALL_FRAMES) const {
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
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, title_,
                               title_pf_, frame)
              && P::get_has_value(nh, journal_,
                                  journal_pf_, frame)
              && P::get_has_value(nh, pubmed_id_,
                                  pubmed_id_pf_, frame)
              && P::get_has_value(nh, year_,
                                  year_pf_, frame)
              && P::get_has_value(nh, authors_,
                                  authors_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, title_,
                                title_pf_, frame)
          && P::get_has_value(nh, journal_,
                              journal_pf_, frame)
          && P::get_has_value(nh, pubmed_id_,
                              pubmed_id_pf_, frame)
          && P::get_has_value(nh, year_,
                              year_pf_, frame)
          && P::get_has_value(nh, authors_,
                              authors_pf_, frame);
      }
    }
  };

  typedef vector<JournalArticleFactory>
  JournalArticleFactories;

  /** Create decorators of type JournalArticle.

      \see JournalArticleConst
      \see JournalArticleFactory
  */
  class JournalArticleConstFactory:
    public Factory<JournalArticleConst>
  {
  private:
    typedef Factory<JournalArticleConst> P;
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
    JournalArticleConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("publication");
        title_=P::get_key<StringTraits>(fh,
                                        cat,
                                        "title",
                                        false);
        title_pf_=P::get_key<StringTraits>(fh,
                                           cat,
                                           "title",
                                           true);
        journal_=P::get_key<StringTraits>(fh,
                                          cat,
                                          "journal",
                                          false);
        journal_pf_=P::get_key<StringTraits>(fh,
                                             cat,
                                             "journal",
                                             true);
        pubmed_id_=P::get_key<StringTraits>(fh,
                                            cat,
                                            "pubmed id",
                                            false);
        pubmed_id_pf_=P::get_key<StringTraits>(fh,
                                               cat,
                                               "pubmed id",
                                               true);
        year_=P::get_key<IntTraits>(fh,
                                    cat,
                                    "year",
                                    false);
        year_pf_=P::get_key<IntTraits>(fh,
                                       cat,
                                       "year",
                                       true);
        authors_=P::get_key<StringsTraits>(fh,
                                           cat,
                                           "authors",
                                           false);
        authors_pf_=P::get_key<StringsTraits>(fh,
                                              cat,
                                              "authors",
                                              true);
      };
    }
    JournalArticleConst get(NodeConstHandle nh,
                            int frame=ALL_FRAMES) const {
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
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, title_,
                               title_pf_, frame)
              && P::get_has_value(nh, journal_,
                                  journal_pf_, frame)
              && P::get_has_value(nh, pubmed_id_,
                                  pubmed_id_pf_, frame)
              && P::get_has_value(nh, year_,
                                  year_pf_, frame)
              && P::get_has_value(nh, authors_,
                                  authors_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, title_,
                                title_pf_, frame)
          && P::get_has_value(nh, journal_,
                              journal_pf_, frame)
          && P::get_has_value(nh, pubmed_id_,
                              pubmed_id_pf_, frame)
          && P::get_has_value(nh, year_,
                              year_pf_, frame)
          && P::get_has_value(nh, authors_,
                              authors_pf_, frame);
      }
    }
  };

  typedef vector<JournalArticleConstFactory>
  JournalArticleConstFactories;

  /** Information regarding a residue.

      \see Residue
      \see ResidueConstFactory
  */
  class ResidueConst:
    public Decorator<ResidueConst, NodeConstHandle> {
    friend class ResidueConstFactory;
  private:
    typedef Decorator<ResidueConst, NodeConstHandle> P;
    boost::array<IntKey,2> index_;
    StringKey type_;
    StringKey type_pf_;
    ResidueConst(NodeConstHandle nh,
                 int frame,
                 boost::array<IntKey, 2> index,
                 StringKey type,
                 StringKey type_pf):
      P(nh, frame),
      index_(index),
      type_(type),
      type_pf_(type_pf) {
      ;
    }
  public:
    Int get_index() const {
      return get_node().get_value(index_[0], get_frame());
    }
    String get_type() const {
      return P::get_value(type_,
                          type_pf_);
    }
    Strings get_all_types() const {
      return P::get_all_values(type_pf_);
    }
    static std::string get_decorator_type_name() {
      return "ResidueConst";
    }
  };

  typedef vector<ResidueConst>
  ResidueConsts;

  /** Information regarding a residue.

      \see ResidueConst
      \see ResidueFactory
  */
  class Residue:
    public Decorator<Residue, NodeHandle> {
    friend class ResidueFactory;
  private:
    typedef Decorator<Residue, NodeHandle> P;
    boost::array<internal::IntLazyKey,2> index_;
    internal::StringLazyKey type_;
    internal::StringLazyKey type_pf_;
    Residue(NodeHandle nh,
            int frame,
            boost::array<internal::IntLazyKey, 2> index,
            internal::StringLazyKey type,
            internal::StringLazyKey type_pf):
      P(nh, frame),
      index_(index),
      type_(type),
      type_pf_(type_pf) {
      ;
    }
  public:
    Int get_index() const {
      return get_node().get_value(index_[0], get_frame());
    }
    void set_index(Int v) {
      get_node().set_value(index_[0], v, get_frame());
      get_node().set_value(index_[1], v, get_frame());
    }
    String get_type() const {
      return P::get_value(type_,
                          type_pf_);
    }
    Strings get_all_types() const {
      return P::get_all_values(type_pf_);
    }
    void set_type(String v) {
      P::set_value(type_,
                   type_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Residue";
    }
  };

  typedef vector<Residue>
  Residues;

  /** Create decorators of type Residue.

      \see Residue
      \see ResidueConstFactory
  */
  class ResidueFactory:
    public Factory<Residue>
  {
  private:
    typedef Factory<Residue> P;
    boost::array<internal::IntLazyKey,2> index_;
    internal::StringLazyKey type_;
    internal::StringLazyKey type_pf_;
  public:
    ResidueFactory(FileHandle fh):
      P(){
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
                int frame=ALL_FRAMES) const {
      ;
      return Residue(nh, frame, index_,
                     type_,
                     type_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (nh.get_has_value(index_[0], frame)
              && nh.get_has_value(index_[1], frame)
              && nh.get_value(index_[0], frame)
              ==nh.get_value(index_[1], frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)) return true;
        }
        return false;
      } else {
        return nh.get_has_value(index_[0], frame)
          && nh.get_has_value(index_[1], frame)
          && nh.get_value(index_[0], frame)
          ==nh.get_value(index_[1], frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame);
      }
    }
  };

  typedef vector<ResidueFactory>
  ResidueFactories;

  /** Create decorators of type Residue.

      \see ResidueConst
      \see ResidueFactory
  */
  class ResidueConstFactory:
    public Factory<ResidueConst>
  {
  private:
    typedef Factory<ResidueConst> P;
    boost::array<IntKey,2> index_;
    StringKey type_;
    StringKey type_pf_;
  public:
    ResidueConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("sequence");
        index_[0]=P::get_key<IntTraits>(fh,
                                        cat,
                                        "first residue index",
                                        false);
        index_[1]=P::get_key<IntTraits>(fh,
                                        cat,
                                        "last residue index",
                                        false);
        type_=P::get_key<StringTraits>(fh,
                                       cat,
                                       "residue type",
                                       false);
        type_pf_=P::get_key<StringTraits>(fh,
                                          cat,
                                          "residue type",
                                          true);
      };
    }
    ResidueConst get(NodeConstHandle nh,
                     int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ResidueConst(nh, frame, index_,
                          type_,
                          type_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (nh.get_has_value(index_[0], frame)
              && nh.get_has_value(index_[1], frame)
              && nh.get_value(index_[0], frame)
              ==nh.get_value(index_[1], frame)
              && P::get_has_value(nh, type_,
                                  type_pf_, frame)) return true;
        }
        return false;
      } else {
        return nh.get_has_value(index_[0], frame)
          && nh.get_has_value(index_[1], frame)
          && nh.get_value(index_[0], frame)
          ==nh.get_value(index_[1], frame)
          && P::get_has_value(nh, type_,
                              type_pf_, frame);
      }
    }
  };

  typedef vector<ResidueConstFactory>
  ResidueConstFactories;

  /** Information regarding an atom.

      \see Atom
      \see AtomConstFactory
  */
  class AtomConst:
    public Decorator<AtomConst, NodeConstHandle> {
    friend class AtomConstFactory;
  private:
    typedef Decorator<AtomConst, NodeConstHandle> P;
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
      P(nh, frame),
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
      return P::get_values(coordinates_, coordinates_pf_);
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    Float get_mass() const {
      return P::get_value(mass_,
                          mass_pf_);
    }
    Floats get_all_masss() const {
      return P::get_all_values(mass_pf_);
    }
    Index get_element() const {
      return P::get_value(element_,
                          element_pf_);
    }
    Indexes get_all_elements() const {
      return P::get_all_values(element_pf_);
    }
    static std::string get_decorator_type_name() {
      return "AtomConst";
    }
  };

  typedef vector<AtomConst>
  AtomConsts;

  /** Information regarding an atom.

      \see AtomConst
      \see AtomFactory
  */
  class Atom:
    public Decorator<Atom, NodeHandle> {
    friend class AtomFactory;
  private:
    typedef Decorator<Atom, NodeHandle> P;
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
      P(nh, frame),
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
      return P::get_values(coordinates_, coordinates_pf_);
    }
    void set_coordinates(const Floats &v) {
      P::set_values(coordinates_, coordinates_pf_, v);
    }
    Float get_radius() const {
      return P::get_value(radius_,
                          radius_pf_);
    }
    Floats get_all_radiuss() const {
      return P::get_all_values(radius_pf_);
    }
    void set_radius(Float v) {
      P::set_value(radius_,
                   radius_pf_, v);
    }
    Float get_mass() const {
      return P::get_value(mass_,
                          mass_pf_);
    }
    Floats get_all_masss() const {
      return P::get_all_values(mass_pf_);
    }
    void set_mass(Float v) {
      P::set_value(mass_,
                   mass_pf_, v);
    }
    Index get_element() const {
      return P::get_value(element_,
                          element_pf_);
    }
    Indexes get_all_elements() const {
      return P::get_all_values(element_pf_);
    }
    void set_element(Index v) {
      P::set_value(element_,
                   element_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Atom";
    }
  };

  typedef vector<Atom>
  Atoms;

  /** Create decorators of type Atom.

      \see Atom
      \see AtomConstFactory
  */
  class AtomFactory:
    public Factory<Atom>
  {
  private:
    typedef Factory<Atom> P;
    internal::FloatLazyKeys coordinates_;
    internal::FloatLazyKeys coordinates_pf_;
    internal::FloatLazyKey radius_;
    internal::FloatLazyKey radius_pf_;
    internal::FloatLazyKey mass_;
    internal::FloatLazyKey mass_pf_;
    internal::IndexLazyKey element_;
    internal::IndexLazyKey element_pf_;
  public:
    AtomFactory(FileHandle fh):
      P(){
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
             int frame=ALL_FRAMES) const {
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
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, mass_,
                                  mass_pf_, frame)
              && P::get_has_value(nh, element_,
                                  element_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, mass_,
                              mass_pf_, frame)
          && P::get_has_value(nh, element_,
                              element_pf_, frame);
      }
    }
  };

  typedef vector<AtomFactory>
  AtomFactories;

  /** Create decorators of type Atom.

      \see AtomConst
      \see AtomFactory
  */
  class AtomConstFactory:
    public Factory<AtomConst>
  {
  private:
    typedef Factory<AtomConst> P;
    FloatKeys coordinates_;
    FloatKeys coordinates_pf_;
    FloatKey radius_;
    FloatKey radius_pf_;
    FloatKey mass_;
    FloatKey mass_pf_;
    IndexKey element_;
    IndexKey element_pf_;
  public:
    AtomConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("physics");
        Strings  coordinates_names;
        coordinates_names.push_back("cartesian x");
        coordinates_names.push_back("cartesian y");
        coordinates_names.push_back("cartesian z");
        coordinates_=P::get_keys<FloatTraits>(fh,
                                              cat,
                                              coordinates_names,
                                              false);
        coordinates_pf_=P::get_keys<FloatTraits>(fh,
                                                 cat,
                                                 coordinates_names,
                                                 true);
        radius_=P::get_key<FloatTraits>(fh,
                                        cat,
                                        "radius",
                                        false);
        radius_pf_=P::get_key<FloatTraits>(fh,
                                           cat,
                                           "radius",
                                           true);
        mass_=P::get_key<FloatTraits>(fh,
                                      cat,
                                      "mass",
                                      false);
        mass_pf_=P::get_key<FloatTraits>(fh,
                                         cat,
                                         "mass",
                                         true);
        element_=P::get_key<IndexTraits>(fh,
                                         cat,
                                         "element",
                                         false);
        element_pf_=P::get_key<IndexTraits>(fh,
                                            cat,
                                            "element",
                                            true);
      };
    }
    AtomConst get(NodeConstHandle nh,
                  int frame=ALL_FRAMES) const {
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
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_values(nh, coordinates_,
                                coordinates_pf_, frame)
              && P::get_has_value(nh, radius_,
                                  radius_pf_, frame)
              && P::get_has_value(nh, mass_,
                                  mass_pf_, frame)
              && P::get_has_value(nh, element_,
                                  element_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_values(nh, coordinates_,
                                 coordinates_pf_, frame)
          && P::get_has_value(nh, radius_,
                              radius_pf_, frame)
          && P::get_has_value(nh, mass_,
                              mass_pf_, frame)
          && P::get_has_value(nh, element_,
                              element_pf_, frame);
      }
    }
  };

  typedef vector<AtomConstFactory>
  AtomConstFactories;

  /** Information regarding a chain.

      \see Chain
      \see ChainConstFactory
  */
  class ChainConst:
    public Decorator<ChainConst, NodeConstHandle> {
    friend class ChainConstFactory;
  private:
    typedef Decorator<ChainConst, NodeConstHandle> P;
    IndexKey chain_id_;
    IndexKey chain_id_pf_;
    ChainConst(NodeConstHandle nh,
               int frame,
               IndexKey chain_id,
               IndexKey chain_id_pf):
      P(nh, frame),
      chain_id_(chain_id),
      chain_id_pf_(chain_id_pf) {
      ;
    }
  public:
    Index get_chain_id() const {
      return P::get_value(chain_id_,
                          chain_id_pf_);
    }
    Indexes get_all_chain_ids() const {
      return P::get_all_values(chain_id_pf_);
    }
    static std::string get_decorator_type_name() {
      return "ChainConst";
    }
  };

  typedef vector<ChainConst>
  ChainConsts;

  /** Information regarding a chain.

      \see ChainConst
      \see ChainFactory
  */
  class Chain:
    public Decorator<Chain, NodeHandle> {
    friend class ChainFactory;
  private:
    typedef Decorator<Chain, NodeHandle> P;
    internal::IndexLazyKey chain_id_;
    internal::IndexLazyKey chain_id_pf_;
    Chain(NodeHandle nh,
          int frame,
          internal::IndexLazyKey chain_id,
          internal::IndexLazyKey chain_id_pf):
      P(nh, frame),
      chain_id_(chain_id),
      chain_id_pf_(chain_id_pf) {
      ;
    }
  public:
    Index get_chain_id() const {
      return P::get_value(chain_id_,
                          chain_id_pf_);
    }
    Indexes get_all_chain_ids() const {
      return P::get_all_values(chain_id_pf_);
    }
    void set_chain_id(Index v) {
      P::set_value(chain_id_,
                   chain_id_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Chain";
    }
  };

  typedef vector<Chain>
  Chains;

  /** Create decorators of type Chain.

      \see Chain
      \see ChainConstFactory
  */
  class ChainFactory:
    public Factory<Chain>
  {
  private:
    typedef Factory<Chain> P;
    internal::IndexLazyKey chain_id_;
    internal::IndexLazyKey chain_id_pf_;
  public:
    ChainFactory(FileHandle fh):
      P(){
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
              int frame=ALL_FRAMES) const {
      ;
      return Chain(nh, frame, chain_id_,
                   chain_id_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, chain_id_,
                               chain_id_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, chain_id_,
                                chain_id_pf_, frame);
      }
    }
  };

  typedef vector<ChainFactory>
  ChainFactories;

  /** Create decorators of type Chain.

      \see ChainConst
      \see ChainFactory
  */
  class ChainConstFactory:
    public Factory<ChainConst>
  {
  private:
    typedef Factory<ChainConst> P;
    IndexKey chain_id_;
    IndexKey chain_id_pf_;
  public:
    ChainConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("sequence");
        chain_id_=P::get_key<IndexTraits>(fh,
                                          cat,
                                          "chain id",
                                          false);
        chain_id_pf_=P::get_key<IndexTraits>(fh,
                                             cat,
                                             "chain id",
                                             true);
      };
    }
    ChainConst get(NodeConstHandle nh,
                   int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ChainConst(nh, frame, chain_id_,
                        chain_id_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, chain_id_,
                               chain_id_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, chain_id_,
                                chain_id_pf_, frame);
      }
    }
  };

  typedef vector<ChainConstFactory>
  ChainConstFactories;

  /** Information regarding a fragment of a molecule.

      \see Domain
      \see DomainConstFactory
  */
  class DomainConst:
    public Decorator<DomainConst, NodeConstHandle> {
    friend class DomainConstFactory;
  private:
    typedef Decorator<DomainConst, NodeConstHandle> P;
    boost::array<IntKey,2> indexes_;
    DomainConst(NodeConstHandle nh,
                int frame,
                boost::array<IntKey, 2> indexes):
      P(nh, frame),
      indexes_(indexes) {
      ;
    }
  public:
    IntRange get_indexes() const {
      return std::make_pair(get_node().get_value(indexes_[0], get_frame()),
                            get_node().get_value(indexes_[1], get_frame()));
    }
    static std::string get_decorator_type_name() {
      return "DomainConst";
    }
  };

  typedef vector<DomainConst>
  DomainConsts;

  /** Information regarding a fragment of a molecule.

      \see DomainConst
      \see DomainFactory
  */
  class Domain:
    public Decorator<Domain, NodeHandle> {
    friend class DomainFactory;
  private:
    typedef Decorator<Domain, NodeHandle> P;
    boost::array<internal::IntLazyKey,2> indexes_;
    Domain(NodeHandle nh,
           int frame,
           boost::array<internal::IntLazyKey, 2> indexes):
      P(nh, frame),
      indexes_(indexes) {
      ;
    }
  public:
    IntRange get_indexes() const {
      return std::make_pair(get_node().get_value(indexes_[0], get_frame()),
                            get_node().get_value(indexes_[1], get_frame()));
    }
    void set_indexes(Int v0, Int v1) {
      get_node().set_value(indexes_[0], v0, get_frame());
      get_node().set_value(indexes_[1], v1, get_frame());
    }
    static std::string get_decorator_type_name() {
      return "Domain";
    }
  };

  typedef vector<Domain>
  Domains;

  /** Create decorators of type Domain.

      \see Domain
      \see DomainConstFactory
  */
  class DomainFactory:
    public Factory<Domain>
  {
  private:
    typedef Factory<Domain> P;
    boost::array<internal::IntLazyKey,2> indexes_;
  public:
    DomainFactory(FileHandle fh):
      P(){
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
               int frame=ALL_FRAMES) const {
      ;
      return Domain(nh, frame, indexes_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (nh.get_has_value(indexes_[0], frame)
              && nh.get_has_value(indexes_[1], frame)
              && nh.get_value(indexes_[0], frame)
              <nh.get_value(indexes_[1], frame)) return true;
        }
        return false;
      } else {
        return nh.get_has_value(indexes_[0], frame)
          && nh.get_has_value(indexes_[1], frame)
          && nh.get_value(indexes_[0], frame)
          <nh.get_value(indexes_[1], frame);
      }
    }
  };

  typedef vector<DomainFactory>
  DomainFactories;

  /** Create decorators of type Domain.

      \see DomainConst
      \see DomainFactory
  */
  class DomainConstFactory:
    public Factory<DomainConst>
  {
  private:
    typedef Factory<DomainConst> P;
    boost::array<IntKey,2> indexes_;
  public:
    DomainConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("sequence");
        indexes_[0]=P::get_key<IntTraits>(fh,
                                          cat,
                                          "first residue index",
                                          false);
        indexes_[1]=P::get_key<IntTraits>(fh,
                                          cat,
                                          "last residue index",
                                          false);
      };
    }
    DomainConst get(NodeConstHandle nh,
                    int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return DomainConst(nh, frame, indexes_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (nh.get_has_value(indexes_[0], frame)
              && nh.get_has_value(indexes_[1], frame)
              && nh.get_value(indexes_[0], frame)
              <nh.get_value(indexes_[1], frame)) return true;
        }
        return false;
      } else {
        return nh.get_has_value(indexes_[0], frame)
          && nh.get_has_value(indexes_[1], frame)
          && nh.get_value(indexes_[0], frame)
          <nh.get_value(indexes_[1], frame);
      }
    }
  };

  typedef vector<DomainConstFactory>
  DomainConstFactories;

  /** Information regarding a copy of a molecule.

      \see Copy
      \see CopyConstFactory
  */
  class CopyConst:
    public Decorator<CopyConst, NodeConstHandle> {
    friend class CopyConstFactory;
  private:
    typedef Decorator<CopyConst, NodeConstHandle> P;
    IndexKey copy_index_;
    IndexKey copy_index_pf_;
    CopyConst(NodeConstHandle nh,
              int frame,
              IndexKey copy_index,
              IndexKey copy_index_pf):
      P(nh, frame),
      copy_index_(copy_index),
      copy_index_pf_(copy_index_pf) {
      ;
    }
  public:
    Index get_copy_index() const {
      return P::get_value(copy_index_,
                          copy_index_pf_);
    }
    Indexes get_all_copy_indexs() const {
      return P::get_all_values(copy_index_pf_);
    }
    static std::string get_decorator_type_name() {
      return "CopyConst";
    }
  };

  typedef vector<CopyConst>
  CopyConsts;

  /** Information regarding a copy of a molecule.

      \see CopyConst
      \see CopyFactory
  */
  class Copy:
    public Decorator<Copy, NodeHandle> {
    friend class CopyFactory;
  private:
    typedef Decorator<Copy, NodeHandle> P;
    internal::IndexLazyKey copy_index_;
    internal::IndexLazyKey copy_index_pf_;
    Copy(NodeHandle nh,
         int frame,
         internal::IndexLazyKey copy_index,
         internal::IndexLazyKey copy_index_pf):
      P(nh, frame),
      copy_index_(copy_index),
      copy_index_pf_(copy_index_pf) {
      ;
    }
  public:
    Index get_copy_index() const {
      return P::get_value(copy_index_,
                          copy_index_pf_);
    }
    Indexes get_all_copy_indexs() const {
      return P::get_all_values(copy_index_pf_);
    }
    void set_copy_index(Index v) {
      P::set_value(copy_index_,
                   copy_index_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Copy";
    }
  };

  typedef vector<Copy>
  Copys;

  /** Create decorators of type Copy.

      \see Copy
      \see CopyConstFactory
  */
  class CopyFactory:
    public Factory<Copy>
  {
  private:
    typedef Factory<Copy> P;
    internal::IndexLazyKey copy_index_;
    internal::IndexLazyKey copy_index_pf_;
  public:
    CopyFactory(FileHandle fh):
      P(){
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
             int frame=ALL_FRAMES) const {
      ;
      return Copy(nh, frame, copy_index_,
                  copy_index_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, copy_index_,
                               copy_index_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, copy_index_,
                                copy_index_pf_, frame);
      }
    }
  };

  typedef vector<CopyFactory>
  CopyFactories;

  /** Create decorators of type Copy.

      \see CopyConst
      \see CopyFactory
  */
  class CopyConstFactory:
    public Factory<CopyConst>
  {
  private:
    typedef Factory<CopyConst> P;
    IndexKey copy_index_;
    IndexKey copy_index_pf_;
  public:
    CopyConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("sequence");
        copy_index_=P::get_key<IndexTraits>(fh,
                                            cat,
                                            "copy index",
                                            false);
        copy_index_pf_=P::get_key<IndexTraits>(fh,
                                               cat,
                                               "copy index",
                                               true);
      };
    }
    CopyConst get(NodeConstHandle nh,
                  int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return CopyConst(nh, frame, copy_index_,
                       copy_index_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, copy_index_,
                               copy_index_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, copy_index_,
                                copy_index_pf_, frame);
      }
    }
  };

  typedef vector<CopyConstFactory>
  CopyConstFactories;

  /** Information regarding diffusion coefficients.

      \see Diffuser
      \see DiffuserConstFactory
  */
  class DiffuserConst:
    public Decorator<DiffuserConst, NodeConstHandle> {
    friend class DiffuserConstFactory;
  private:
    typedef Decorator<DiffuserConst, NodeConstHandle> P;
    FloatKey diffusion_coefficient_;
    FloatKey diffusion_coefficient_pf_;
    DiffuserConst(NodeConstHandle nh,
                  int frame,
                  FloatKey diffusion_coefficient,
                  FloatKey diffusion_coefficient_pf):
      P(nh, frame),
      diffusion_coefficient_(diffusion_coefficient),
      diffusion_coefficient_pf_(diffusion_coefficient_pf) {
      ;
    }
  public:
    Float get_diffusion_coefficient() const {
      return P::get_value(diffusion_coefficient_,
                          diffusion_coefficient_pf_);
    }
    Floats get_all_diffusion_coefficients() const {
      return P::get_all_values(diffusion_coefficient_pf_);
    }
    static std::string get_decorator_type_name() {
      return "DiffuserConst";
    }
  };

  typedef vector<DiffuserConst>
  DiffuserConsts;

  /** Information regarding diffusion coefficients.

      \see DiffuserConst
      \see DiffuserFactory
  */
  class Diffuser:
    public Decorator<Diffuser, NodeHandle> {
    friend class DiffuserFactory;
  private:
    typedef Decorator<Diffuser, NodeHandle> P;
    internal::FloatLazyKey diffusion_coefficient_;
    internal::FloatLazyKey diffusion_coefficient_pf_;
    Diffuser(NodeHandle nh,
             int frame,
             internal::FloatLazyKey diffusion_coefficient,
             internal::FloatLazyKey diffusion_coefficient_pf):
      P(nh, frame),
      diffusion_coefficient_(diffusion_coefficient),
      diffusion_coefficient_pf_(diffusion_coefficient_pf) {
      ;
    }
  public:
    Float get_diffusion_coefficient() const {
      return P::get_value(diffusion_coefficient_,
                          diffusion_coefficient_pf_);
    }
    Floats get_all_diffusion_coefficients() const {
      return P::get_all_values(diffusion_coefficient_pf_);
    }
    void set_diffusion_coefficient(Float v) {
      P::set_value(diffusion_coefficient_,
                   diffusion_coefficient_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Diffuser";
    }
  };

  typedef vector<Diffuser>
  Diffusers;

  /** Create decorators of type Diffuser.

      \see Diffuser
      \see DiffuserConstFactory
  */
  class DiffuserFactory:
    public Factory<Diffuser>
  {
  private:
    typedef Factory<Diffuser> P;
    internal::FloatLazyKey diffusion_coefficient_;
    internal::FloatLazyKey diffusion_coefficient_pf_;
  public:
    DiffuserFactory(FileHandle fh):
      P(){
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
                 int frame=ALL_FRAMES) const {
      ;
      return Diffuser(nh, frame, diffusion_coefficient_,
                      diffusion_coefficient_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, diffusion_coefficient_,
                               diffusion_coefficient_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, diffusion_coefficient_,
                                diffusion_coefficient_pf_, frame);
      }
    }
  };

  typedef vector<DiffuserFactory>
  DiffuserFactories;

  /** Create decorators of type Diffuser.

      \see DiffuserConst
      \see DiffuserFactory
  */
  class DiffuserConstFactory:
    public Factory<DiffuserConst>
  {
  private:
    typedef Factory<DiffuserConst> P;
    FloatKey diffusion_coefficient_;
    FloatKey diffusion_coefficient_pf_;
  public:
    DiffuserConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("physics");
        diffusion_coefficient_=P::get_key<FloatTraits>(fh,
                                                       cat,
                                                       "diffusion coefficient",
                                                       false);
        diffusion_coefficient_pf_=P::get_key<FloatTraits>(fh,
                                                          cat,
                                                       "diffusion coefficient",
                                                          true);
      };
    }
    DiffuserConst get(NodeConstHandle nh,
                      int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return DiffuserConst(nh, frame, diffusion_coefficient_,
                           diffusion_coefficient_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, diffusion_coefficient_,
                               diffusion_coefficient_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, diffusion_coefficient_,
                                diffusion_coefficient_pf_, frame);
      }
    }
  };

  typedef vector<DiffuserConstFactory>
  DiffuserConstFactories;

  /** A numeric tag for keeping track of types of molecules.

      \see Typed
      \see TypedConstFactory
  */
  class TypedConst:
    public Decorator<TypedConst, NodeConstHandle> {
    friend class TypedConstFactory;
  private:
    typedef Decorator<TypedConst, NodeConstHandle> P;
    StringKey type_name_;
    StringKey type_name_pf_;
    TypedConst(NodeConstHandle nh,
               int frame,
               StringKey type_name,
               StringKey type_name_pf):
      P(nh, frame),
      type_name_(type_name),
      type_name_pf_(type_name_pf) {
      ;
    }
  public:
    String get_type_name() const {
      return P::get_value(type_name_,
                          type_name_pf_);
    }
    Strings get_all_type_names() const {
      return P::get_all_values(type_name_pf_);
    }
    static std::string get_decorator_type_name() {
      return "TypedConst";
    }
  };

  typedef vector<TypedConst>
  TypedConsts;

  /** A numeric tag for keeping track of types of molecules.

      \see TypedConst
      \see TypedFactory
  */
  class Typed:
    public Decorator<Typed, NodeHandle> {
    friend class TypedFactory;
  private:
    typedef Decorator<Typed, NodeHandle> P;
    internal::StringLazyKey type_name_;
    internal::StringLazyKey type_name_pf_;
    Typed(NodeHandle nh,
          int frame,
          internal::StringLazyKey type_name,
          internal::StringLazyKey type_name_pf):
      P(nh, frame),
      type_name_(type_name),
      type_name_pf_(type_name_pf) {
      ;
    }
  public:
    String get_type_name() const {
      return P::get_value(type_name_,
                          type_name_pf_);
    }
    Strings get_all_type_names() const {
      return P::get_all_values(type_name_pf_);
    }
    void set_type_name(String v) {
      P::set_value(type_name_,
                   type_name_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Typed";
    }
  };

  typedef vector<Typed>
  Typeds;

  /** Create decorators of type Typed.

      \see Typed
      \see TypedConstFactory
  */
  class TypedFactory:
    public Factory<Typed>
  {
  private:
    typedef Factory<Typed> P;
    internal::StringLazyKey type_name_;
    internal::StringLazyKey type_name_pf_;
  public:
    TypedFactory(FileHandle fh):
      P(){
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
              int frame=ALL_FRAMES) const {
      ;
      return Typed(nh, frame, type_name_,
                   type_name_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, type_name_,
                               type_name_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, type_name_,
                                type_name_pf_, frame);
      }
    }
  };

  typedef vector<TypedFactory>
  TypedFactories;

  /** Create decorators of type Typed.

      \see TypedConst
      \see TypedFactory
  */
  class TypedConstFactory:
    public Factory<TypedConst>
  {
  private:
    typedef Factory<TypedConst> P;
    StringKey type_name_;
    StringKey type_name_pf_;
  public:
    TypedConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("sequence");
        type_name_=P::get_key<StringTraits>(fh,
                                            cat,
                                            "type name",
                                            false);
        type_name_pf_=P::get_key<StringTraits>(fh,
                                               cat,
                                               "type name",
                                               true);
      };
    }
    TypedConst get(NodeConstHandle nh,
                   int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return TypedConst(nh, frame, type_name_,
                        type_name_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, type_name_,
                               type_name_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, type_name_,
                                type_name_pf_, frame);
      }
    }
  };

  typedef vector<TypedConstFactory>
  TypedConstFactories;

  /** Store a reference to another node as an alias.

      \see Alias
      \see AliasConstFactory
  */
  class AliasConst:
    public Decorator<AliasConst, NodeConstHandle> {
    friend class AliasConstFactory;
  private:
    typedef Decorator<AliasConst, NodeConstHandle> P;
    NodeIDKey aliased_;
    NodeIDKey aliased_pf_;
    AliasConst(NodeConstHandle nh,
               int frame,
               NodeIDKey aliased,
               NodeIDKey aliased_pf):
      P(nh, frame),
      aliased_(aliased),
      aliased_pf_(aliased_pf) {
      ;
    }
  public:
    NodeConstHandle get_aliased() const {
      NodeID id;
      if (get_node().get_has_value(aliased_)) {
        id= get_node().get_value(aliased_);
      } else {
        id= get_node().get_value(aliased_pf_, get_frame());
      }
      return get_node().get_file().get_node_from_id(id);
    }
    static std::string get_decorator_type_name() {
      return "AliasConst";
    }
  };

  typedef vector<AliasConst>
  AliasConsts;

  /** Store a reference to another node as an alias.

      \see AliasConst
      \see AliasFactory
  */
  class Alias:
    public Decorator<Alias, NodeHandle> {
    friend class AliasFactory;
  private:
    typedef Decorator<Alias, NodeHandle> P;
    internal::NodeIDLazyKey aliased_;
    internal::NodeIDLazyKey aliased_pf_;
    Alias(NodeHandle nh,
          int frame,
          internal::NodeIDLazyKey aliased,
          internal::NodeIDLazyKey aliased_pf):
      P(nh, frame),
      aliased_(aliased),
      aliased_pf_(aliased_pf) {
      ;
    }
  public:
    NodeHandle get_aliased() const {
      NodeID id;
      if (get_node().get_has_value(aliased_)) {
        id= get_node().get_value(aliased_);
      } else {
        id= get_node().get_value(aliased_pf_, get_frame());
      }
      return get_node().get_file().get_node_from_id(id);
    }
    void set_aliased(NodeConstHandle v) {
      if (get_frame() >=0) {
        get_node().set_value(aliased_pf_, v.get_id(), get_frame());
      } else {
        return get_node().set_value(aliased_, v.get_id());
      }
    }
    static std::string get_decorator_type_name() {
      return "Alias";
    }
  };

  typedef vector<Alias>
  Aliass;

  /** Create decorators of type Alias.

      \see Alias
      \see AliasConstFactory
  */
  class AliasFactory:
    public Factory<Alias>
  {
  private:
    typedef Factory<Alias> P;
    internal::NodeIDLazyKey aliased_;
    internal::NodeIDLazyKey aliased_pf_;
  public:
    AliasFactory(FileHandle fh):
      P(){
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
    Alias get(NodeHandle nh,
              int frame=ALL_FRAMES) const {
      ;
      return Alias(nh, frame, aliased_,
                   aliased_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, aliased_,
                               aliased_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, aliased_,
                                aliased_pf_, frame);
      }
    }
  };

  typedef vector<AliasFactory>
  AliasFactories;

  /** Create decorators of type Alias.

      \see AliasConst
      \see AliasFactory
  */
  class AliasConstFactory:
    public Factory<AliasConst>
  {
  private:
    typedef Factory<AliasConst> P;
    NodeIDKey aliased_;
    NodeIDKey aliased_pf_;
  public:
    AliasConstFactory(FileConstHandle fh):
      P(){
      {
        CategoryD<1> cat=fh.get_category<1>("alias");
        aliased_=P::get_key<NodeIDTraits>(fh,
                                          cat,
                                          "aliased",
                                          false);
        aliased_pf_=P::get_key<NodeIDTraits>(fh,
                                             cat,
                                             "aliased",
                                             true);
      };
    }
    AliasConst get(NodeConstHandle nh,
                   int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return AliasConst(nh, frame, aliased_,
                        aliased_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, aliased_,
                               aliased_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, aliased_,
                                aliased_pf_, frame);
      }
    }
  };

  typedef vector<AliasConstFactory>
  AliasConstFactories;

  /** Associate a score with some set of particles.

      \see Score
      \see ScoreConstFactory
  */
  class ScoreConst:
    public Decorator<ScoreConst, NodeConstHandle> {
    friend class ScoreConstFactory;
  private:
    typedef Decorator<ScoreConst, NodeConstHandle> P;
    AliasConstFactory representation_;
    FloatKey score_;
    FloatKey score_pf_;
    ScoreConst(NodeConstHandle nh,
               int frame,
               AliasConstFactory representation,
               FloatKey score,
               FloatKey score_pf):
      P(nh, frame),
      representation_(representation),
      score_(score),
      score_pf_(score_pf) {
      ;
    }
  public:
    NodeConstHandles get_representation() const {
      NodeConstHandles typed=get_node().get_children();
      NodeConstHandles ret;
      for (unsigned int i=0; i< typed.size(); ++i) {
        if (representation_.get_is(typed[i])) {
          ret.push_back(representation_.get(typed[i]).get_aliased());
        }
      }
      return ret;
    }
    Float get_score() const {
      return P::get_value(score_,
                          score_pf_);
    }
    Floats get_all_scores() const {
      return P::get_all_values(score_pf_);
    }
    static std::string get_decorator_type_name() {
      return "ScoreConst";
    }
  };

  typedef vector<ScoreConst>
  ScoreConsts;

  /** Associate a score with some set of particles.

      \see ScoreConst
      \see ScoreFactory
  */
  class Score:
    public Decorator<Score, NodeHandle> {
    friend class ScoreFactory;
  private:
    typedef Decorator<Score, NodeHandle> P;
    AliasFactory representation_;
    internal::FloatLazyKey score_;
    internal::FloatLazyKey score_pf_;
    Score(NodeHandle nh,
          int frame,
          AliasFactory representation,
          internal::FloatLazyKey score,
          internal::FloatLazyKey score_pf):
      P(nh, frame),
      representation_(representation),
      score_(score),
      score_pf_(score_pf) {
      ;
    }
  public:
    NodeHandles get_representation() const {
      NodeHandles typed=get_node().get_children();
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
        add_child_alias(get_node(), v[i]);
      }
    }
    void set_representation(NodeHandles v) {
      for (unsigned int i=0; i< v.size(); ++i) {
        add_child_alias(get_node(), v[i]);
      }
    }
    Float get_score() const {
      return P::get_value(score_,
                          score_pf_);
    }
    Floats get_all_scores() const {
      return P::get_all_values(score_pf_);
    }
    void set_score(Float v) {
      P::set_value(score_,
                   score_pf_, v);
    }
    static std::string get_decorator_type_name() {
      return "Score";
    }
  };

  typedef vector<Score>
  Scores;

  /** Create decorators of type Score.

      \see Score
      \see ScoreConstFactory
  */
  class ScoreFactory:
    public Factory<Score>
  {
  private:
    typedef Factory<Score> P;
    AliasFactory representation_;
    internal::FloatLazyKey score_;
    internal::FloatLazyKey score_pf_;
  public:
    ScoreFactory(FileHandle fh):
      P(), representation_(fh){
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
              int frame=ALL_FRAMES) const {
      ;
      return Score(nh, frame, representation_,
                   score_,
                   score_pf_);
    }
    bool get_is(NodeHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, score_,
                               score_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, score_,
                                score_pf_, frame);
      }
    }
  };

  typedef vector<ScoreFactory>
  ScoreFactories;

  /** Create decorators of type Score.

      \see ScoreConst
      \see ScoreFactory
  */
  class ScoreConstFactory:
    public Factory<ScoreConst>
  {
  private:
    typedef Factory<ScoreConst> P;
    AliasConstFactory representation_;
    FloatKey score_;
    FloatKey score_pf_;
  public:
    ScoreConstFactory(FileConstHandle fh):
      P(), representation_(fh){
      {
        CategoryD<1> cat=fh.get_category<1>("feature");
        score_=P::get_key<FloatTraits>(fh,
                                       cat,
                                       "score",
                                       false);
        score_pf_=P::get_key<FloatTraits>(fh,
                                          cat,
                                          "score",
                                          true);
      };
    }
    ScoreConst get(NodeConstHandle nh,
                   int frame=ALL_FRAMES) const {
      IMP_RMF_USAGE_CHECK(get_is(nh, frame), "Node is not");
      return ScoreConst(nh, frame, representation_,
                        score_,
                        score_pf_);
    }
    bool get_is(NodeConstHandle nh, int frame= ALL_FRAMES) const {
      if (frame == ANY_FRAME) {
        int num_frames=nh.get_file().get_number_of_frames();
        for (frame=0; frame< num_frames; ++frame) {
          if (P::get_has_value(nh, score_,
                               score_pf_, frame)) return true;
        }
        return false;
      } else {
        return P::get_has_value(nh, score_,
                                score_pf_, frame);
      }
    }
  };

  typedef vector<ScoreConstFactory>
  ScoreConstFactories;

} /* namespace RMF */

#endif /* IMPLIBRMF_DECORATORS_H */
