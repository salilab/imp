#ifndef _PARTICLESACCESSPOINT_H
#define _PARTICLESACCESSPOINT_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT
  Handles access to particles 

*/
class ParticlesAccessPoint {
public:
  // implmenets particle iterator
  virtual ~ParticlesAccessPoint(){}
  virtual int get_size() const =0;
  virtual float get_x(int index)const=0;
  virtual float get_y(int index)const=0;
  virtual float get_z(int index)const=0;
  virtual float get_r(int index)const=0;
  virtual float get_w(int index)const=0;

};
#endif //_PARTICLESACCESSPOINT_H
