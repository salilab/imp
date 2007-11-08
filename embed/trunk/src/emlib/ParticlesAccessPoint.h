#ifndef _PARTICLESACCESSPOINT_H
#define _PARTICLESACCESSPOINT_H



#include <iostream>
#include <iomanip>

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
