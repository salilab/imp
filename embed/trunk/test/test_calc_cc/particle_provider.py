from EM import ParticlesAccessPoint
#implement the particle access point class
class particles_provider(ParticlesAccessPoint):
	def read(self,fn):
		self.particles=[map(float,line.split()) for line in file(fn) if(line[0]!='#')]
	def get_size(self):
		return len(self.particles)
	def get_x(self,i):
		return self.particles[i][0]
	def get_y(self,i):
		return self.particles[i][1]
	def get_z(self,i):
		return self.particles[i][2]
	def get_r(self,i):
		return self.particles[i][3]
	def get_w(self,i):
		return self.particles[i][4]
	def translate(self,x,y,z):
		for p in self.particles:
			p[0]=p[0]+x;p[1]=p[1]+y;p[2]=p[2]+z;
	

