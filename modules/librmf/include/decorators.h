/**
 *  ile RMF/decorators.h
 *  rief Helper functions for manipulating RMF files.
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
friend class ColoredConstFactory;
private:
FloatKeys rgb_color_;
ColoredConst(NodeConstHandle nh,
              FloatKeys rgb_color): nh_(nh),rgb_color_(rgb_color) {
;
}
public:
Floats get_rgb_color(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(rgb_color_[i], frame));
   }
   return ret;
}
IMP_RMF_SHOWABLE(ConstColored, "ColoredConst " << nh_.get_name());
~ColoredConst() {
;
}
};

typedef vector<ColoredConst> ColoredConsts;

/** These particles has associated color information.

   \see ColoredConst
   \see ColoredFactory
 */
class Colored {
NodeHandle nh_;
friend class ColoredFactory;
private:
FloatKeys rgb_color_;
Colored(NodeHandle nh,
              FloatKeys rgb_color): nh_(nh),rgb_color_(rgb_color) {
;
}
public:
Floats get_rgb_color(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(rgb_color_[i], frame));
   }
   return ret;
}
void set_rgb_color(const Floats &v, int frame=0) {
   for (unsigned int i=0; i< 3; ++i) {
      nh_.set_value(rgb_color_[i], v[i], frame);
   }
}
IMP_RMF_SHOWABLE(ConstColored, "Colored " << nh_.get_name());
~Colored() {
;
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
IMP_RMF_SHOWABLE(ColoredFactory, "Factory");
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
Colored get(NodeHandle nh) const {
  return Colored(nh, rgb_color_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(rgb_color_[0]);
}
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
IMP_RMF_SHOWABLE(ColoredConstFactory, "Factory");
ColoredConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("shape");
      rgb_color_.push_back((fh.get_has_key<FloatTraits>(cat, "rgb color red")?
               fh.get_key<FloatTraits>(cat, "rgb color red"):FloatKey()));
      rgb_color_.push_back((fh.get_has_key<FloatTraits>(cat, "rgb color green")?
               fh.get_key<FloatTraits>(cat, "rgb color green"):FloatKey()));
      rgb_color_.push_back((fh.get_has_key<FloatTraits>(cat, "rgb color blue")?
               fh.get_key<FloatTraits>(cat, "rgb color blue"):FloatKey()));;
};
}
ColoredConst get(NodeConstHandle nh) const {
  return ColoredConst(nh, rgb_color_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(rgb_color_[0]);
}
};

typedef vector<ColoredConstFactory> ColoredConstFactories;

/** These particles has associated coordinates and radius information.

   \see Particle
   \see ParticleConstFactory
 */
class ParticleConst {
NodeConstHandle nh_;
friend class ParticleConstFactory;
private:
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
ParticleConst(NodeConstHandle nh,
              FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),coordinates_(coordinates),
radius_(radius),
mass_(mass) {
;
}
public:
Floats get_coordinates(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
Float get_mass(int frame=0) const {
  return nh_.get_value(mass_, frame);
}
IMP_RMF_SHOWABLE(ConstParticle, "ParticleConst " << nh_.get_name());
~ParticleConst() {
;
}
};

typedef vector<ParticleConst> ParticleConsts;

/** These particles has associated coordinates and radius information.

   \see ParticleConst
   \see ParticleFactory
 */
class Particle {
NodeHandle nh_;
friend class ParticleFactory;
private:
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
Particle(NodeHandle nh,
              FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),coordinates_(coordinates),
radius_(radius),
mass_(mass) {
;
}
public:
Floats get_coordinates(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
void set_coordinates(const Floats &v, int frame=0) {
   for (unsigned int i=0; i< 3; ++i) {
      nh_.set_value(coordinates_[i], v[i], frame);
   }
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
void set_radius(Float v, int frame=0) {
   nh_.set_value(radius_, v, frame);
}
Float get_mass(int frame=0) const {
  return nh_.get_value(mass_, frame);
}
void set_mass(Float v, int frame=0) {
   nh_.set_value(mass_, v, frame);
}
IMP_RMF_SHOWABLE(ConstParticle, "Particle " << nh_.get_name());
~Particle() {
;
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
IMP_RMF_SHOWABLE(ParticleFactory, "Factory");
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
Particle get(NodeHandle nh) const {
  return Particle(nh, coordinates_,
radius_,
mass_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_)
    && nh.get_has_value(mass_);
}
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
IMP_RMF_SHOWABLE(ParticleConstFactory, "Factory");
ParticleConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("physics");
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
               fh.get_key<FloatTraits>(cat, "cartesian x"):FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
               fh.get_key<FloatTraits>(cat, "cartesian y"):FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
               fh.get_key<FloatTraits>(cat, "cartesian z"):FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
               fh.get_key<FloatTraits>(cat, "radius"):FloatKey());
mass_=(fh.get_has_key<FloatTraits>(cat, "mass")?
               fh.get_key<FloatTraits>(cat, "mass"):FloatKey());
};
}
ParticleConst get(NodeConstHandle nh) const {
  return ParticleConst(nh, coordinates_,
radius_,
mass_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_)
    && nh.get_has_value(mass_);
}
};

typedef vector<ParticleConstFactory> ParticleConstFactories;

/** These particles has associated coordinates and radius information.

   \see RigidParticle
   \see RigidParticleConstFactory
 */
class RigidParticleConst {
NodeConstHandle nh_;
friend class RigidParticleConstFactory;
private:
FloatKeys orientation_;
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
RigidParticleConst(NodeConstHandle nh,
              FloatKeys orientation,
FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),orientation_(orientation),
coordinates_(coordinates),
radius_(radius),
mass_(mass) {
;
}
public:
Floats get_orientation(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 4; ++i) {
      ret.push_back(nh_.get_value(orientation_[i], frame));
   }
   return ret;
}
Floats get_coordinates(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
Float get_mass(int frame=0) const {
  return nh_.get_value(mass_, frame);
}
IMP_RMF_SHOWABLE(ConstRigidParticle, "RigidParticleConst " << nh_.get_name());
~RigidParticleConst() {
;
}
};

typedef vector<RigidParticleConst> RigidParticleConsts;

/** These particles has associated coordinates and radius information.

   \see RigidParticleConst
   \see RigidParticleFactory
 */
class RigidParticle {
NodeHandle nh_;
friend class RigidParticleFactory;
private:
FloatKeys orientation_;
FloatKeys coordinates_;
FloatKey radius_;
FloatKey mass_;
RigidParticle(NodeHandle nh,
              FloatKeys orientation,
FloatKeys coordinates,
FloatKey radius,
FloatKey mass): nh_(nh),orientation_(orientation),
coordinates_(coordinates),
radius_(radius),
mass_(mass) {
;
}
public:
Floats get_orientation(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 4; ++i) {
      ret.push_back(nh_.get_value(orientation_[i], frame));
   }
   return ret;
}
void set_orientation(const Floats &v, int frame=0) {
   for (unsigned int i=0; i< 4; ++i) {
      nh_.set_value(orientation_[i], v[i], frame);
   }
}
Floats get_coordinates(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
void set_coordinates(const Floats &v, int frame=0) {
   for (unsigned int i=0; i< 3; ++i) {
      nh_.set_value(coordinates_[i], v[i], frame);
   }
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
void set_radius(Float v, int frame=0) {
   nh_.set_value(radius_, v, frame);
}
Float get_mass(int frame=0) const {
  return nh_.get_value(mass_, frame);
}
void set_mass(Float v, int frame=0) {
   nh_.set_value(mass_, v, frame);
}
IMP_RMF_SHOWABLE(ConstRigidParticle, "RigidParticle " << nh_.get_name());
~RigidParticle() {
;
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
IMP_RMF_SHOWABLE(RigidParticleFactory, "Factory");
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
RigidParticle get(NodeHandle nh) const {
  return RigidParticle(nh, orientation_,
coordinates_,
radius_,
mass_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(orientation_[0])
    && nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_)
    && nh.get_has_value(mass_);
}
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
IMP_RMF_SHOWABLE(RigidParticleConstFactory, "Factory");
RigidParticleConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("physics");
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation r")?
               fh.get_key<FloatTraits>(cat, "orientation r"):FloatKey()));
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation i")?
               fh.get_key<FloatTraits>(cat, "orientation i"):FloatKey()));
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation j")?
               fh.get_key<FloatTraits>(cat, "orientation j"):FloatKey()));
      orientation_.push_back((fh.get_has_key<FloatTraits>(cat, "orientation k")?
               fh.get_key<FloatTraits>(cat, "orientation k"):FloatKey()));;
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
               fh.get_key<FloatTraits>(cat, "cartesian x"):FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
               fh.get_key<FloatTraits>(cat, "cartesian y"):FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
               fh.get_key<FloatTraits>(cat, "cartesian z"):FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
               fh.get_key<FloatTraits>(cat, "radius"):FloatKey());
mass_=(fh.get_has_key<FloatTraits>(cat, "mass")?
               fh.get_key<FloatTraits>(cat, "mass"):FloatKey());
};
}
RigidParticleConst get(NodeConstHandle nh) const {
  return RigidParticleConst(nh, orientation_,
coordinates_,
radius_,
mass_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(orientation_[0])
    && nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_)
    && nh.get_has_value(mass_);
}
};

typedef vector<RigidParticleConstFactory> RigidParticleConstFactories;

/** Associate a score with some set of particles.

   \see Score
   \see ScoreConstFactory
 */
class ScoreConst {
NodeConstHandle nh_;
friend class ScoreConstFactory;
private:
NodeIDsKey representation_;
FloatKey score_;
ScoreConst(NodeConstHandle nh,
              NodeIDsKey representation,
FloatKey score): nh_(nh),representation_(representation),
score_(score) {
;
}
public:
NodeIDs get_representation(int frame=0) const {
  return nh_.get_value(representation_, frame);
}
Float get_score(int frame=0) const {
  return nh_.get_value(score_, frame);
}
IMP_RMF_SHOWABLE(ConstScore, "ScoreConst " << nh_.get_name());
~ScoreConst() {
;
}
};

typedef vector<ScoreConst> ScoreConsts;

/** Associate a score with some set of particles.

   \see ScoreConst
   \see ScoreFactory
 */
class Score {
NodeHandle nh_;
friend class ScoreFactory;
private:
NodeIDsKey representation_;
FloatKey score_;
Score(NodeHandle nh,
              NodeIDsKey representation,
FloatKey score): nh_(nh),representation_(representation),
score_(score) {
;
}
public:
NodeIDs get_representation(int frame=0) const {
  return nh_.get_value(representation_, frame);
}
void set_representation(NodeIDs v, int frame=0) {
   nh_.set_value(representation_, v, frame);
}
Float get_score(int frame=0) const {
  return nh_.get_value(score_, frame);
}
void set_score(Float v, int frame=0) {
   nh_.set_value(score_, v, frame);
}
IMP_RMF_SHOWABLE(ConstScore, "Score " << nh_.get_name());
~Score() {
;
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
IMP_RMF_SHOWABLE(ScoreFactory, "Factory");
ScoreFactory(FileHandle fh){
{
  CategoryD<1> cat=get_category_always<1>(fh, "feature");
representation_=get_key_always<NodeIDsTraits>(fh, cat,
                           "representation", false);
score_=get_key_always<FloatTraits>(fh, cat,
                           "score", false);
};
}
Score get(NodeHandle nh) const {
  return Score(nh, representation_,
score_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(representation_)
    && nh.get_has_value(score_);
}
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
IMP_RMF_SHOWABLE(ScoreConstFactory, "Factory");
ScoreConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("feature");
representation_=(fh.get_has_key<NodeIDsTraits>(cat, "representation")?
               fh.get_key<NodeIDsTraits>(cat, "representation"):NodeIDsKey());
score_=(fh.get_has_key<FloatTraits>(cat, "score")?
               fh.get_key<FloatTraits>(cat, "score"):FloatKey());
};
}
ScoreConst get(NodeConstHandle nh) const {
  return ScoreConst(nh, representation_,
score_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(representation_)
    && nh.get_has_value(score_);
}
};

typedef vector<ScoreConstFactory> ScoreConstFactories;

/** A geometric ball.

   \see Ball
   \see BallConstFactory
 */
class BallConst {
NodeConstHandle nh_;
friend class BallConstFactory;
private:
FloatKeys coordinates_;
FloatKey radius_;
BallConst(NodeConstHandle nh,
              FloatKeys coordinates,
FloatKey radius): nh_(nh),coordinates_(coordinates),
radius_(radius) {
;
}
public:
Floats get_coordinates(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
IMP_RMF_SHOWABLE(ConstBall, "BallConst " << nh_.get_name());
~BallConst() {
;
}
};

typedef vector<BallConst> BallConsts;

/** A geometric ball.

   \see BallConst
   \see BallFactory
 */
class Ball {
NodeHandle nh_;
friend class BallFactory;
private:
FloatKeys coordinates_;
FloatKey radius_;
Ball(NodeHandle nh,
              FloatKeys coordinates,
FloatKey radius): nh_(nh),coordinates_(coordinates),
radius_(radius) {
;
}
public:
Floats get_coordinates(int frame=0) const {
   Floats ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
void set_coordinates(const Floats &v, int frame=0) {
   for (unsigned int i=0; i< 3; ++i) {
      nh_.set_value(coordinates_[i], v[i], frame);
   }
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
void set_radius(Float v, int frame=0) {
   nh_.set_value(radius_, v, frame);
}
IMP_RMF_SHOWABLE(ConstBall, "Ball " << nh_.get_name());
~Ball() {
;
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
public:
IMP_RMF_SHOWABLE(BallFactory, "Factory");
BallFactory(FileHandle fh){
{
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                           "cartesian x", false));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                           "cartesian y", false));
      coordinates_.push_back(get_key_always<FloatTraits>(fh, cat,
                           "cartesian z", false));;
radius_=get_key_always<FloatTraits>(fh, cat,
                           "radius", false);
};
}
Ball get(NodeHandle nh) const {
  return Ball(nh, coordinates_,
radius_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_);
}
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
public:
IMP_RMF_SHOWABLE(BallConstFactory, "Factory");
BallConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("shape");
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian x")?
               fh.get_key<FloatTraits>(cat, "cartesian x"):FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian y")?
               fh.get_key<FloatTraits>(cat, "cartesian y"):FloatKey()));
      coordinates_.push_back((fh.get_has_key<FloatTraits>(cat, "cartesian z")?
               fh.get_key<FloatTraits>(cat, "cartesian z"):FloatKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
               fh.get_key<FloatTraits>(cat, "radius"):FloatKey());
};
}
BallConst get(NodeConstHandle nh) const {
  return BallConst(nh, coordinates_,
radius_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_);
}
};

typedef vector<BallConstFactory> BallConstFactories;

/** A geometric cylinder.

   \see Cylinder
   \see CylinderConstFactory
 */
class CylinderConst {
NodeConstHandle nh_;
friend class CylinderConstFactory;
private:
FloatsKeys coordinates_;
FloatKey radius_;
CylinderConst(NodeConstHandle nh,
              FloatsKeys coordinates,
FloatKey radius): nh_(nh),coordinates_(coordinates),
radius_(radius) {
;
}
public:
FloatsList get_coordinates(int frame=0) const {
   FloatsList ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
IMP_RMF_SHOWABLE(ConstCylinder, "CylinderConst " << nh_.get_name());
~CylinderConst() {
;
}
};

typedef vector<CylinderConst> CylinderConsts;

/** A geometric cylinder.

   \see CylinderConst
   \see CylinderFactory
 */
class Cylinder {
NodeHandle nh_;
friend class CylinderFactory;
private:
FloatsKeys coordinates_;
FloatKey radius_;
Cylinder(NodeHandle nh,
              FloatsKeys coordinates,
FloatKey radius): nh_(nh),coordinates_(coordinates),
radius_(radius) {
;
}
public:
FloatsList get_coordinates(int frame=0) const {
   FloatsList ret;
   for (unsigned int i=0; i< 3; ++i) {
      ret.push_back(nh_.get_value(coordinates_[i], frame));
   }
   return ret;
}
void set_coordinates(const FloatsList &v, int frame=0) {
   for (unsigned int i=0; i< 3; ++i) {
      nh_.set_value(coordinates_[i], v[i], frame);
   }
}
Float get_radius(int frame=0) const {
  return nh_.get_value(radius_, frame);
}
void set_radius(Float v, int frame=0) {
   nh_.set_value(radius_, v, frame);
}
IMP_RMF_SHOWABLE(ConstCylinder, "Cylinder " << nh_.get_name());
~Cylinder() {
;
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
public:
IMP_RMF_SHOWABLE(CylinderFactory, "Factory");
CylinderFactory(FileHandle fh){
{
  CategoryD<1> cat=get_category_always<1>(fh, "shape");
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                           "cartesian xs", false));
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                           "cartesian ys", false));
      coordinates_.push_back(get_key_always<FloatsTraits>(fh, cat,
                           "cartesian zs", false));;
radius_=get_key_always<FloatTraits>(fh, cat,
                           "radius", false);
};
}
Cylinder get(NodeHandle nh) const {
  return Cylinder(nh, coordinates_,
radius_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_);
}
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
public:
IMP_RMF_SHOWABLE(CylinderConstFactory, "Factory");
CylinderConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("shape");
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian xs")?
               fh.get_key<FloatsTraits>(cat, "cartesian xs"):FloatsKey()));
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian ys")?
               fh.get_key<FloatsTraits>(cat, "cartesian ys"):FloatsKey()));
      coordinates_.push_back((fh.get_has_key<FloatsTraits>(cat, "cartesian zs")?
               fh.get_key<FloatsTraits>(cat, "cartesian zs"):FloatsKey()));;
radius_=(fh.get_has_key<FloatTraits>(cat, "radius")?
               fh.get_key<FloatTraits>(cat, "radius"):FloatKey());
};
}
CylinderConst get(NodeConstHandle nh) const {
  return CylinderConst(nh, coordinates_,
radius_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(coordinates_[0])
    && nh.get_has_value(radius_);
}
};

typedef vector<CylinderConstFactory> CylinderConstFactories;

/** Information regarding a publication.

   \see JournalArticle
   \see JournalArticleConstFactory
 */
class JournalArticleConst {
NodeConstHandle nh_;
friend class JournalArticleConstFactory;
private:
StringKey title_;
StringKey journal_;
StringKey pubmed_id_;
IntKey year_;
StringsKey authors_;
JournalArticleConst(NodeConstHandle nh,
              StringKey title,
StringKey journal,
StringKey pubmed_id,
IntKey year,
StringsKey authors): nh_(nh),title_(title),
journal_(journal),
pubmed_id_(pubmed_id),
year_(year),
authors_(authors) {
;
}
public:
String get_title(int frame=0) const {
  return nh_.get_value(title_, frame);
}
String get_journal(int frame=0) const {
  return nh_.get_value(journal_, frame);
}
String get_pubmed_id(int frame=0) const {
  return nh_.get_value(pubmed_id_, frame);
}
Int get_year(int frame=0) const {
  return nh_.get_value(year_, frame);
}
Strings get_authors(int frame=0) const {
  return nh_.get_value(authors_, frame);
}
IMP_RMF_SHOWABLE(ConstJournalArticle, "JournalArticleConst " << nh_.get_name());
~JournalArticleConst() {
;
}
};

typedef vector<JournalArticleConst> JournalArticleConsts;

/** Information regarding a publication.

   \see JournalArticleConst
   \see JournalArticleFactory
 */
class JournalArticle {
NodeHandle nh_;
friend class JournalArticleFactory;
private:
StringKey title_;
StringKey journal_;
StringKey pubmed_id_;
IntKey year_;
StringsKey authors_;
JournalArticle(NodeHandle nh,
              StringKey title,
StringKey journal,
StringKey pubmed_id,
IntKey year,
StringsKey authors): nh_(nh),title_(title),
journal_(journal),
pubmed_id_(pubmed_id),
year_(year),
authors_(authors) {
;
}
public:
String get_title(int frame=0) const {
  return nh_.get_value(title_, frame);
}
void set_title(String v, int frame=0) {
   nh_.set_value(title_, v, frame);
}
String get_journal(int frame=0) const {
  return nh_.get_value(journal_, frame);
}
void set_journal(String v, int frame=0) {
   nh_.set_value(journal_, v, frame);
}
String get_pubmed_id(int frame=0) const {
  return nh_.get_value(pubmed_id_, frame);
}
void set_pubmed_id(String v, int frame=0) {
   nh_.set_value(pubmed_id_, v, frame);
}
Int get_year(int frame=0) const {
  return nh_.get_value(year_, frame);
}
void set_year(Int v, int frame=0) {
   nh_.set_value(year_, v, frame);
}
Strings get_authors(int frame=0) const {
  return nh_.get_value(authors_, frame);
}
void set_authors(Strings v, int frame=0) {
   nh_.set_value(authors_, v, frame);
}
IMP_RMF_SHOWABLE(ConstJournalArticle, "JournalArticle " << nh_.get_name());
~JournalArticle() {
;
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
IMP_RMF_SHOWABLE(JournalArticleFactory, "Factory");
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
JournalArticle get(NodeHandle nh) const {
  return JournalArticle(nh, title_,
journal_,
pubmed_id_,
year_,
authors_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(title_)
    && nh.get_has_value(journal_)
    && nh.get_has_value(pubmed_id_)
    && nh.get_has_value(year_)
    && nh.get_has_value(authors_);
}
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
IMP_RMF_SHOWABLE(JournalArticleConstFactory, "Factory");
JournalArticleConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("publication");
title_=(fh.get_has_key<StringTraits>(cat, "title")?
               fh.get_key<StringTraits>(cat, "title"):StringKey());
journal_=(fh.get_has_key<StringTraits>(cat, "journal")?
               fh.get_key<StringTraits>(cat, "journal"):StringKey());
pubmed_id_=(fh.get_has_key<StringTraits>(cat, "pubmed id")?
               fh.get_key<StringTraits>(cat, "pubmed id"):StringKey());
year_=(fh.get_has_key<IntTraits>(cat, "year")?
               fh.get_key<IntTraits>(cat, "year"):IntKey());
authors_=(fh.get_has_key<StringsTraits>(cat, "authors")?
               fh.get_key<StringsTraits>(cat, "authors"):StringsKey());
};
}
JournalArticleConst get(NodeConstHandle nh) const {
  return JournalArticleConst(nh, title_,
journal_,
pubmed_id_,
year_,
authors_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(title_)
    && nh.get_has_value(journal_)
    && nh.get_has_value(pubmed_id_)
    && nh.get_has_value(year_)
    && nh.get_has_value(authors_);
}
};

typedef vector<JournalArticleConstFactory> JournalArticleConstFactories;

/** Information regarding a residue.

   \see Residue
   \see ResidueConstFactory
 */
class ResidueConst {
NodeConstHandle nh_;
friend class ResidueConstFactory;
private:
boost::array<IntKey,2> index_;
StringKey type_;
ResidueConst(NodeConstHandle nh,
              boost::array<IntKey, 2> index,
StringKey type): nh_(nh),index_(index),
type_(type) {
;
}
public:
Int get_index(int frame=0) const {
  return nh_.get_value(index_[0], frame);
}
String get_type(int frame=0) const {
  return nh_.get_value(type_, frame);
}
IMP_RMF_SHOWABLE(ConstResidue, "ResidueConst " << nh_.get_name());
~ResidueConst() {
;
}
};

typedef vector<ResidueConst> ResidueConsts;

/** Information regarding a residue.

   \see ResidueConst
   \see ResidueFactory
 */
class Residue {
NodeHandle nh_;
friend class ResidueFactory;
private:
boost::array<IntKey,2> index_;
StringKey type_;
Residue(NodeHandle nh,
              boost::array<IntKey, 2> index,
StringKey type): nh_(nh),index_(index),
type_(type) {
;
}
public:
Int get_index(int frame=0) const {
  return nh_.get_value(index_[0], frame);
}
void set_index(Int v, int frame=0) {
   nh_.set_value(index_[0], v, frame);
   nh_.set_value(index_[0], v, frame);
}
String get_type(int frame=0) const {
  return nh_.get_value(type_, frame);
}
void set_type(String v, int frame=0) {
   nh_.set_value(type_, v, frame);
}
IMP_RMF_SHOWABLE(ConstResidue, "Residue " << nh_.get_name());
~Residue() {
;
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
IMP_RMF_SHOWABLE(ResidueFactory, "Factory");
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
Residue get(NodeHandle nh) const {
  return Residue(nh, index_,
type_);
}
bool get_is(NodeHandle nh) const {
  return nh.get_has_value(index_[0])
  && nh.get_has_value(index_[1])
  && nh.get_value(index_[0])==nh.get_value(index_[1])
    && nh.get_has_value(type_);
}
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
IMP_RMF_SHOWABLE(ResidueConstFactory, "Factory");
ResidueConstFactory(FileConstHandle fh){
{
  CategoryD<1> cat=fh.get_category<1>("sequence");
index_[0]=(fh.get_has_key<IntTraits>(cat, "first residue index")?
               fh.get_key<IntTraits>(cat, "first residue index"):IntKey());
index_[1]=(fh.get_has_key<IntTraits>(cat, "last residue index")?
               fh.get_key<IntTraits>(cat, "last residue index"):IntKey());
type_=(fh.get_has_key<StringTraits>(cat, "residue type")?
               fh.get_key<StringTraits>(cat, "residue type"):StringKey());
};
}
ResidueConst get(NodeConstHandle nh) const {
  return ResidueConst(nh, index_,
type_);
}
bool get_is(NodeConstHandle nh) const {
  return nh.get_has_value(index_[0])
  && nh.get_has_value(index_[1])
  && nh.get_value(index_[0])==nh.get_value(index_[1])
    && nh.get_has_value(type_);
}
};

typedef vector<ResidueConstFactory> ResidueConstFactories;

} /* namespace RMF */

#endif /* IMPLIBRMF_DECORATORS_H */
