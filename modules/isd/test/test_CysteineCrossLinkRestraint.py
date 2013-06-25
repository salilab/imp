#!/usr/bin/env python

#general imports
import gzip
from math import exp,log
from random import sample

#imp general
import IMP

#our project
from IMP.isd import Nuisance,Scale,CysteineCrossLinkRestraint,CysteineCrossLinkData,CrossLinkData

#unit testing framework
import IMP.test


class TestCysteineCrossLinkRestraint(IMP.test.TestCase):

    def get_cross_link_data(self,directory,filename,(distmin,distmax,ndist),
                                             (omegamin,omegamax,nomega),
                                            (sigmamin,sigmamax,nsigma)):
      dist_grid=self.get_grid(distmin, distmax, ndist, False)
      omega_grid=self.get_log_grid(omegamin, omegamax, nomega)
      sigma_grid=self.get_log_grid(sigmamin, sigmamax, nsigma)
      #try to open the cross-link database    
      filen=IMP.isd.get_data_path("CrossLinkPMFs.dict")
      xlpot=open(filen)

      for line in xlpot:
        dictionary=eval(line)
        break
      xpot=dictionary[directory][filename]["distance"]
      pot=dictionary[directory][filename]["gofr"]

      xlmsdata=IMP.isd.CrossLinkData(dist_grid,omega_grid,sigma_grid,xpot,pot,10.0,20.0)
      return xlmsdata

    def setup_nuisance(self,m,initialvalue,minvalue,maxvalue,isoptimized=True):
        nuisance=IMP.isd.Scale.setup_particle(IMP.Particle(m),initialvalue)
        nuisance.set_lower(minvalue)
        nuisance.set_upper(maxvalue)
        nuisance.set_is_optimized(nuisance.get_nuisance_key(),isoptimized)
        return nuisance
        
    def setup_weight(self,m,isoptimized=True):
        pw=IMP.Particle(m)
        weight=IMP.isd.Weight.setup_particle(pw)
        weight.set_weights_are_optimized(True)
        return weight

    def get_grid(self,gmin,gmax,ngrid,boundaries):
        grid=[]
        dx = ( gmax - gmin ) / float(ngrid)
        for i in range(0,ngrid+1):
           if(not boundaries and i==0): continue
           if(not boundaries and i==ngrid): continue
           grid.append( gmin + float(i) * dx )
        return grid

    def get_log_grid(self,gmin,gmax,ngrid):
        grid=[]
        for i in range(0,ngrid+1):
           grid.append( gmin*exp(float(i)/ngrid*log(gmax/gmin)) )
        return grid
    
    def testSetupAndEvaluate(self):
       "test the restraint on precalculated parameter grid"
       m=IMP.Model()

       ps=[]
       #setting up particles
       p0=IMP.Particle(m)
       xyz=IMP.core.XYZR.setup_particle(p0,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),3.0))
       xyz.set_coordinates_are_optimized(True)
       ps.append(p0)

       p0=IMP.Particle(m)
       xyz0=IMP.core.XYZR.setup_particle(p0,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),3.0))
       xyz0.set_coordinates_are_optimized(True)
       ps.append(p0)

       p0=IMP.Particle(m)
       xyz0=IMP.core.XYZR.setup_particle(p0,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),3.0))
       xyz0.set_coordinates_are_optimized(True)
       ps.append(p0)

 
       disttuple=(0.0,25.0, 1000)
       omegatuple=(1.0, 1000.0, 50)
       sigmatuple=(1.0,1.0,1)

       crossdata=self.get_cross_link_data("cysteine","cysteine_CA_FES.txt.standard",
                  disttuple,omegatuple,sigmatuple)

       betatuple=(0.03, 0.1,30) 
       epsilontuple=(0.01,0.99)
       # beta
       beta=self.setup_nuisance(m,betatuple[0],betatuple[0],betatuple[1],True)
       # sigma
       sigma=self.setup_nuisance(m,sigmatuple[0],sigmatuple[0],sigmatuple[1],True)
       #population particle
       weight=self.setup_weight(m,True)
       #epsilon
       epsilon=self.setup_nuisance(m,epsilontuple[0],epsilontuple[0],epsilontuple[1],True)

       # create grids needed by CysteineCrossLinkData
       fmod_grid=self.get_grid(0.0, 1.0, 300, True)
       omega2_grid=self.get_log_grid(0.001, 10000.0, 100)
       beta_grid=self.get_log_grid(betatuple[0],betatuple[1],betatuple[2])

       restdict={}
        
       for fexp in [0,0.25,0.5,0.75,1.0]:
           #generate a number of restraints for each data point value
           datacyst=IMP.isd.CysteineCrossLinkData(fexp,fmod_grid,omega2_grid,beta_grid)
           cystrest=IMP.isd.CysteineCrossLinkRestraint(beta,sigma,epsilon,weight,crossdata,datacyst)
           cystrest.add_contribution(ps[0],ps[1])
           cystrest.add_contribution(ps[0],ps[2])
           restdict[fexp]=cystrest

       f = gzip.open(IMP.isd.get_data_path('test_CysteineCrosslink.data.gz'), 'rb')
       
       testlist=[]
       
       for l in f:
           
           #structure of file entries:
           #fexp,x1,x2,ibeta,iweight,iepsilon,cystrest.unprotected_evaluate(None),cystrest.get_model_frequency()
           t=eval(l)
           testlist.append(t)
           
       for t in sample(testlist,int(len(testlist)*0.1)):
           IMP.core.XYZ(ps[1]).set_coordinates(IMP.algebra.Vector3D(t[1],0.0,0.0))
           IMP.core.XYZ(ps[2]).set_coordinates(IMP.algebra.Vector3D(t[2],0.0,0.0))           
           beta.set_scale(t[3])
           weight.set_weights([t[4],1-t[4]])
           epsilon.set_scale(t[5])
           
           self.assertAlmostEqual(restdict[t[0]].unprotected_evaluate(None),
                    t[6],delta=0.000001)
           self.assertAlmostEqual(restdict[t[0]].get_model_frequency(),
                    t[7],delta=0.000001)
        
if __name__ == '__main__':
    IMP.test.main()
