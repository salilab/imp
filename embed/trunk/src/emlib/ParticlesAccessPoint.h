#ifndef _PARTICLESACCESSPOINT_H
#define _PARTICLESACCESSPOINT_H



#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "EM_config.h"

class EMDLLEXPORT ParticlesAccessPoint {
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




class EMDLLEXPORT particles_provider : public ParticlesAccessPoint {
public:
	std::vector<float> x,y,z,radii,weights;
	
	~particles_provider(){}
	particles_provider(void){}
	// The constructor reads the file
	particles_provider(const char *filename)
	{
		read(filename);
	}

	// Translate function
	void translate(float xo,float yo,float zo)
	{
		int n = get_size();
		for(int i=0;i<n;i++)
		{
			x[i]+=xo;y[i]+=yo;z[i]+=zo;
		}	
		
	}

	void read(const char *filename)
	{
		std::fstream fs;
		int i,ncd;
		float x_,y_,z_,r_,w_;

		fs.open(filename, std::fstream::in );
		if(!fs.fail())
		{
			fs >> ncd; // Read number of coordinates
			// Get values
			for(i=0;i<ncd;i++)
			{
				fs >> x_ >> y_ >> z_ >> r_ >> w_;
				append(x_,y_,z_,r_,w_);
			}	
		}
		fs.close();
	}

	void  append(float x_,float y_,float z_,float r_,float w_) 
	{
		x.push_back(x_);
		y.push_back(y_);
		z.push_back(z_);
		radii.push_back(r_);
		weights.push_back(w_);
	}

	int get_size() const {return x.size();}
	float get_x(int index)const {return x[index];}
	float get_y(int index)const {return y[index];}
	float get_z(int index)const {return z[index];}
	float get_r(int index)const {return radii[index];}
	float get_w(int index)const {return weights[index];}
};








#endif //_PARTICLESACCESSPOINT_H
