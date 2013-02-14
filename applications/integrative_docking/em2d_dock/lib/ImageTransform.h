/**
 *  \file ImageTransform  \brief stores image transformation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_IMAGE_TRANSFORM_H
#define IMP_IMAGE_TRANSFORM_H

#include <IMP/algebra/constants.h>

#include <math.h>
#include <iostream>

#define IMP_RAD_2_DEG(a) 180*a/IMP::algebra::PI
#define IMP_DEG_2_RAD(a) a*IMP::algebra::PI/180

class ImageTransform {
public:
  // Constructors
  ImageTransform() : angle_(0.0), x_(0), y_(0), score_(0.0), area_score_(0.0) {}
  ImageTransform(double angle) :
    angle_(angle), x_(0), y_(0), score_(0.0), area_score_(0.0) {}
  ImageTransform(double angle, int x, int y) :
    angle_(angle), x_(x), y_(y), score_(0.0), area_score_(0.0) {}
  ImageTransform(double angle, int x, int y, double score) :
    angle_(angle), x_(x), y_(y), score_(score), area_score_(0.0) {}
  ImageTransform(double angle, int x, int y, double score, double area_score) :
    angle_(angle), x_(x), y_(y), score_(score), area_score_(area_score) {}
  ImageTransform(int x, int y) :
    angle_(0.0), x_(x), y_(y), score_(0.0), area_score_(0.0) {}
  ImageTransform(int x, int y, double score) :
    angle_(0.0), x_(x), y_(y), score_(score), area_score_(0.0) {}

  // Access
  double get_angle() const { return angle_; }
  int get_x() const { return x_; }
  int get_y() const { return y_; }
  double get_score() const { return score_; }
  double get_area_score() const { return area_score_; }

  // Modifiers
  void set_angle(double angle) { angle_ = angle; }
  void set_x(int x) { x_ = x; }
  void set_y(int y) { y_ = y; }
  void set_score(double score) { score_ = score; }
  void set_area_score(double score) { area_score_ = score; }

  friend std::ostream& operator<<(std::ostream& s, const ImageTransform &t) {
    return s << IMP_RAD_2_DEG(t.angle_) << ' ' << t.x_ << ' '
             << t.y_ << ' ' << t.score_ << ' ' << t.area_score_;
  }

protected:
  double angle_; // radians
  int x_;
  int y_;
  double score_;
  double area_score_;
};

#endif /* IMP_IMAGE_TRANSFORM_H */
